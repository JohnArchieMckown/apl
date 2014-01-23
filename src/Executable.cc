/*
    This file is part of GNU APL, a free implementation of the
    ISO/IEC Standard 13751, "Programming Language APL, Extended"

    Copyright (C) 2008-2013  Dr. Jürgen Sauermann

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Executable.hh"
#include "Output.hh"
#include "PrintOperator.hh"
#include "UCS_string.hh"
#include "UserFunction.hh"
#include "Value.hh"
#include "Workspace.hh"

//-----------------------------------------------------------------------------
Executable::Executable(const UCS_string & ucs, const char * loc)
   : alloc_loc(loc)
{
// { cerr << "Executable " << (void *)this << " created at " << loc << endl; }
   // initialize text, stripping carriage returns and line feeds.
   //
UCS_string line;

   loop(t, ucs.size())
      {
        const Unicode uni = ucs[t];
        switch (uni)
           {
             case UNI_ASCII_CR: break;;
             case UNI_ASCII_LF: text.push_back(line);   line.clear();   break;
             default: line.append(uni);
          }
      }

   if (line.size())   text.push_back(line);
}
//-----------------------------------------------------------------------------
Executable::Executable(Fun_signature sig, const UCS_string & fname,
           const UCS_string & lambda_text, const char * loc)
   : alloc_loc(loc)
{
   text.push_back(UserFunction_header::lambda_header(sig, fname));
   text.push_back(lambda_text);
}
//-----------------------------------------------------------------------------
Executable::~Executable()
{
   Log(LOG_UserFunction__fix)
      {
        CERR << "deleting Executable " << (const void *)this
             << " (body size=" <<  body.size() << ")" << endl;
      }

   loop(b, body.size())   body[b].extract_apl_val(LOC);
}
//-----------------------------------------------------------------------------
void
Executable::parse_body_line(Function_Line line, const UCS_string & ucs_line,
                            const char * loc)
{
   Log(LOG_UserFunction__set_line)
      CERR << "[" << line << "]" << ucs_line << endl;

Token_string in;
const Parser parser(get_parse_mode(), loc);
   {
     ErrorCode ec = parser.parse(ucs_line, in);
     if (ec != E_NO_ERROR)   throw_parse_error(ec, LOC, LOC);
   }

   parse_body_line(line, in, loc);
} 
//-----------------------------------------------------------------------------
void
Executable::parse_body_line(Function_Line line, const Token_string & in,
                            const char * loc)
{
Source<Token> src(in);

   // handle labs (if any)
   //
   if (get_parse_mode() == PM_FUNCTION &&
       src.rest() >= 2                 &&
       src[0].get_tag() == TOK_SYMBOL  &&
       src[1].get_tag() == TOK_COLON)
      {
        Token tok = src.get();   // get the label symbol
        ++src;                   // skip the :

        UserFunction * ufun = get_ufun();
        Assert(ufun);
        ufun->add_label(tok.get_sym_ptr(), line);
      }

Token_string out;
   while (src.rest())
      {
        uint32_t stat_len = src.rest();   // assume statement == line

        // find next diamond. If there is one, adjust stat_len accordingly.
        //
        loop(t, src.rest())
            {
              if (src[t].get_tag() == TOK_DIAMOND)
                 {
                   stat_len = t;
                   break;
                 }
            }

        Source<Token> statement(src, 0, stat_len);
        src.skip(stat_len + 1);

        // copy in reverse order
        //
        while (statement.rest())
            {
              // check for invalid token. The only token allowed are
              // permanent token and { } for lambdas (which will be removed
              // next)
              //
              const Token & tok = statement.get_last();
              if (tok.get_Class() >= TC_MAX_PERM &&
                  tok.get_tag() != TOK_L_CURLY &&
                  tok.get_tag() != TOK_R_CURLY)
                 {
                   CERR << "Line " << line << endl
                        << "Offending token: (tag > TC_MAX_PERM) "
                        << tok.get_tag() << " " << tok << endl
                        << "Statement: ";
                   Token::print_token_list(CERR, statement);
                   SYNTAX_ERROR;
                 }

               out.append(Token());
               move_2(out[out.size() - 1], tok, LOC);
            }


        // add an appropriate end token.
        //
        if (get_parse_mode() == PM_EXECUTE)   ;
        else if (out.size())                  out.append(Token(TOK_ENDL), LOC);
      }

   Log(LOG_UserFunction__set_line)
      {
        CERR << "[non-reverse " << line << "] ";
        Parser::print_token_list(CERR, out, 0);
      } 

   loop(t, out.size())   body.append(out[t], LOC);
}
//-----------------------------------------------------------------------------
Token
Executable::execute_body() const
{
StateIndicator & si = *Workspace::SI_top();

   try                 { return si.run();                         }
   catch (Error err)   { return Token(TOK_ERROR, err.error_code); }
   catch (Token tok)   { return tok;                              }
   catch (...)         { return Token(TOK_ERROR, E_SYSTEM_ERROR); }

   // not reached
   FIXME;
}
//-----------------------------------------------------------------------------
ostream &
Executable::print(ostream & out) const
{
   out << endl
       <<  "Function body [" << body.size() << " token]:" << endl;

   Parser::print_token_list(out, body);
   return out;
}
//-----------------------------------------------------------------------------
UCS_string
Executable::statement_text(Function_PC pc) const
{
   if (pc >= body.size())
      {
        Assert(pc == body.size());
        pc = Function_PC(body.size() - 1);
      }

   if (pc > 0 && body[pc].get_Class() == TC_RETURN)   pc = Function_PC(pc - 1);
   if (pc > 0 && body[pc].get_Class() == TC_END)      pc = Function_PC(pc - 1);

const Function_Line line = get_line(pc);
const UCS_string & line_txt = text[line];

   // count statement ends before pc
   //
int stats_before = 0;
   for (int p = line_start(line); p < pc; ++p)
       {
         if (body[p].get_Class() == TC_END)               ++stats_before;
         else if (body[p].get_tag() == TOK_RETURN_EXEC)   ++stats_before;
       }

int tidx = 0;
   while (stats_before)
      {
        Assert(tidx < line_txt.size());
        if (line_txt[tidx++] == UNI_DIAMOND)   --stats_before;
      }

   // skip leading spaces
   //
   while (line_txt[tidx] == ' ')   ++tidx;

UCS_string ret;
   while (tidx < line_txt.size())
      {
        if (line_txt[tidx] == UNI_DIAMOND)    break;
        ret.append(line_txt[tidx++], LOC);
      }

   // skip trailing spaces
   //
   while (ret.size() && ret.back() <= ' ')   ret.pop_back();

   return ret;
}
//-----------------------------------------------------------------------------
void
Executable::set_error_info(Error & error, Function_PC2 pc_from_to) const
{
const ErrorCode ec = error.error_code;
UCS_string & message_2 = error.error_message_2;

   // for value errors we point to the failed symbol itself.
   //
   if (ec == E_VALUE_ERROR)   pc_from_to.low = pc_from_to.high;

   Log(LOG_prefix__location_info)
      {
        Q1(pc_from_to.low)
        Q1(pc_from_to.high)
      }

   if (body[pc_from_to.high].get_Class() == TC_RETURN &&
       pc_from_to.high > pc_from_to.low)
       pc_from_to.high = Function_PC(pc_from_to.high - 1);;

/*
    The statement is stored in reverse order in the body:

    Body:  
          start ... pc_low      ...       pc_high ... end
                    ^                     ^
                    |                     |
                    +--- failed action ---+

    Display:
          end ... pc_high      ...      pc_low ... start ◊ next statement
                  ^                     ^
                  |                     |
                  +--- failed action ---+
 */

const Function_PC start = get_statement_start(pc_from_to.low);
Function_PC end = start;
   while (body[end].get_Class() != TC_END &&
          body[end].get_tag() != TOK_RETURN_EXEC)   ++end;

   Assert(start   <= pc_from_to.low);
   Assert(pc_from_to.low  <= pc_from_to.high);
   Assert(pc_from_to.high <= end);

   // if the error was caused by some function being executed then
   // the right arg of the function was OK and we skip it
   //
   if (body[pc_from_to.low].get_Class() == TC_VALUE)
      {
         ++pc_from_to.low;
      }

   Log(LOG_prefix__location_info)
      {
        Q1(start)
        Q1(pc_from_to.low)
        Q1(pc_from_to.high)
        Q1(end)
      }

   // Line 2: statement
   //
int len_left = 0;      // characters before the first caret
int len_between = 0;   // distance between the carets
   for (int q = end - 1; q >= start; --q)
       {
         // Note: Token::error_info returns -len if it inserts a space.
         // Such an inserted space counts for the previous token.
         // The previous token is q + 1 since we count down.
         //
         int len = body[q].error_info(message_2);

         if (len < 0)   // space inserted, len is negative
            {
              if ((q + 1) > pc_from_to.high)       len_left++;
              else if ((q + 1) > pc_from_to.low)   len_between++;
              len = -len;
            }

         if (q > pc_from_to.high)         len_left += len;
         else if (q > pc_from_to.low)   len_between += len;
       }

   // Line 3: carets
   //
   error.left_caret += len_left;

   if (pc_from_to.high != pc_from_to.low)   // two carets
      {
        error.right_caret = error.left_caret + len_between;
      }
}
//-----------------------------------------------------------------------------
Function_PC
Executable::get_statement_start(int pc) const
{
   // this function is used in error reporting so it should
   // not Assert() and the like to avoid infinite recursion.

   if (pc >= body.size())   pc = body.size() - 1;

   // if we are at the end of the statement, move back.
   //
   if (pc < 2)   return Function_PC_0;
   if (body[pc].get_Class() == TC_RETURN)   --pc;
   if (body[pc].get_Class() == TC_END)      --pc;

   for (; pc > 0; --pc)
      {
        if (body[pc].get_Class() == TC_END)          return Function_PC(pc + 1);
        if (body[pc].get_tag() == TOK_RETURN_EXEC)   return Function_PC(pc + 1);
      }

   return Function_PC_0;
}
//-----------------------------------------------------------------------------
void
Executable::unmark_all_values() const
{
   loop (b, body.size())
      {
        const Token & tok = body[b];
        if (tok.get_ValueType() != TV_VAL)      continue;

        Value_P value = tok.get_apl_val();
        if (!!value)   value->clear_marked();
      }
}
//-----------------------------------------------------------------------------
int
Executable::show_owners(const char * prefix, ostream & out, const Value & value) const
{
int count = 0;

   loop (b, body.size())
      {
        const Token & tok = body[b];
        if (tok.get_ValueType() != TV_VAL)      continue;

        if (tok.get_apl_val().get() == &value)
           {
             out << prefix << get_name() << "[" << b << "]" << endl;
             ++count;
           }
      }

   return count;
}
//-----------------------------------------------------------------------------
void
Executable::setup_lambdas()
{
int tidx = 0;
int tcol = 0;

   loop(b, body.size())
       {
         // body is in reverse order, so } comes before { (and optional ← NAME)
         //
         if (body[b].get_tag() != TOK_R_CURLY)   continue;   // not {

         body[b++].clear(LOC);   // invalidate }
         Token_string lambda_body;

         int signature = SIG_NONE;

         for (bool goon = true; goon;)
             {
               Assert(b < body.size());
               Token t;
               move_1(t, body[b], LOC);
               body[b].clear(LOC);   // invalidate in main body

               switch(t.get_tag())
                  {
                    case TOK_L_CURLY:  goon = false;         continue;

                    case TOK_ALPHA:    signature |= SIG_A;    /* no break */
                    case TOK_OMEGA:    signature |= SIG_B;    break;
                    case TOK_CHI:      signature |= SIG_X;    break;
                    case TOK_OMEGA_U:  signature |= SIG_RO;   /* no break */
                    case TOK_ALPHA_U:  signature |= SIG_LO;   break;

                    case TOK_R_CURLY:  DEFN_ERROR;
                    case TOK_DIAMOND:  DEFN_ERROR;
                    case TOK_BRANCH:   DEFN_ERROR;
                    case TOK_ESCAPE:   DEFN_ERROR;
                  }

               lambda_body.append(t, LOC);
               ++b;
             }

         if (lambda_body.size())   signature |= SIG_Z;

         Token_string rev_lambda_body;
         if (signature & SIG_Z)
              {
                Token ret_lambda(TOK_RETURN_SYMBOL, &Workspace::get_v_LAMBDA());
                rev_lambda_body.append(ret_lambda, LOC);

                rev_lambda_body.append(Token(TOK_ENDL));

                Symbol * sym_Z = &Workspace::get_v_LAMBDA();
                rev_lambda_body.append(Token(TOK_LAMBDA, sym_Z), LOC);
                rev_lambda_body.append(Token(TOK_ASSIGN), LOC);

                loop(r, lambda_body.size())
                    rev_lambda_body.append(
                                    lambda_body[lambda_body.size() - r - 1]);
              }
           else
              {
                Token ret_void(TOK_RETURN_VOID);
                rev_lambda_body.append(ret_void, LOC);
              }


         // at this point {} was copied from body to rev_lambda_body and
         // b is at the (now invalidated) { token.
         // check if lambda is named, i.e. Name ← { ... }
         //
         UCS_string lambda_name;
         Symbol * lambda_sym = 0;
         if (   (b + 2) < body.size()
             && body[b + 1].get_tag() == TOK_ASSIGN
             && body[b + 2].get_tag() == TOK_LSYMB)
            {
              // named lambda
              //
              lambda_sym = body[b + 2].get_sym_ptr();
              lambda_name = lambda_sym->get_name();
              signature |= SIG_FUN;

               body[b + 1].clear(LOC);
               body[b + 2].clear(LOC);
            }
         else
            {
              lambda_name.append(UNI_LAMBDA);
              lambda_name.append_number(unnamed_lambdas.size() + 1);
            }

         UCS_string lambda_text;
         if (signature & SIG_Z)   lambda_text.append_utf8("λ←");

         int lambda_skip = unnamed_lambdas.size() + named_lambdas.size();
         for (;;)
             {
               // search { in body text
               //
               Assert(tidx < text.size());
               const UCS_string & line = text[tidx];

               if (tcol >= line.size())   // end of line: wrap to next line
                  {
                    ++tidx;
                    tcol = 0;
                    continue;
                  }

               const Unicode l_curly = line[tcol++];
               if (l_curly != UNI_ASCII_L_CURLY)   continue;   // not {

                // found { (but maybe from a previous lambda)
                //
                if (lambda_skip)   { --lambda_skip;   continue; }

                if (tcol >= line.size())   SYNTAX_ERROR;
                for (;;)
                    {
                      const Unicode r_curly = line[tcol++];
                      if (r_curly == UNI_ASCII_R_CURLY)   break;
                      lambda_text.append(r_curly);
                    }
               break;
             }

         UserFunction * ufun = new UserFunction(Fun_signature(signature),
                                                lambda_name, lambda_text,
                                                rev_lambda_body);

         if (signature & SIG_FUN)   // named lambda
            {
              named_lambdas.push_back(ufun);

              // replace the lambda by its name, but as committed value
              // so that it doesn't print (similar to ⎕FX).
              //
              Value_P val_name(new Value(lambda_name, LOC));
              move_2(body[b], Token(TOK_APL_VALUE2, val_name), LOC);
            }
         else                       // unnamed lambda
            {
              unnamed_lambdas.push_back(ufun);
              move_2(body[b], ufun->get_token(), LOC);
            }
       }

   Parser::remove_void_token(body);
}
//=============================================================================
ExecuteList *
ExecuteList::fix(const UCS_string & data, const char * loc)
{
   // clear errors that may have occured before
   {
     Error * err = Workspace::get_error();
     if (err) err->error_code = E_NO_ERROR;
   }

ExecuteList * fun = new ExecuteList(data, loc);

   Log(LOG_UserFunction__fix)
      {
        CERR << "fix pmode=execute list:" << endl << data
             << " addr " << (const void *)fun << endl
             << "------------------- fix --" << endl;
      }

   {
     Error * err = Workspace::get_error();
     if (err && err->error_code)
        {
          Log(LOG_UserFunction__fix)
             {
                CERR << "fix pmode=execute list failed with error "
                     << Error::error_name(err->error_code) << endl;
             }

          err->parser_loc = 0;
          delete fun;
          return 0;
        }
   }

   try
      {
        fun->parse_body_line(Function_Line_0, data, loc);
      }
   catch (Error err)
      {
        Error * werr = Workspace::get_error();
        if (werr)   *werr = err;
        delete fun;
        return 0;
      }

   Log(LOG_UserFunction__fix)
      {
        CERR << "fun->body.size() is " << fun->body.size() << endl;
      }

   // for ⍎ we don't append TOK_END, but only TOK_RETURN_EXEC.
   fun->body.append(Token(TOK_RETURN_EXEC), LOC);

   Log(LOG_UserFunction__fix)   fun->print(CERR);
   return fun;
}
//=============================================================================
StatementList *
StatementList::fix(const UCS_string & data, const char * loc)
{
StatementList * fun = new StatementList(data, loc);

   Log(LOG_UserFunction__fix)
      {
        CERR << "fix pmode=statement list:" << endl << data << endl
             << " addr " << (const void *)fun << endl
             << "------------------- fix --" << endl;
      }

   {
     Error * err = Workspace::get_error();
     if (err)   err->parser_loc = 0;
   }

   fun->parse_body_line(Function_Line_0, data, loc);
   fun->setup_lambdas();

   Log(LOG_UserFunction__fix)
      {
        CERR << "fun->body.size() is " << fun->body.size() << endl;
      }

   fun->body.append(Token(TOK_RETURN_STATS), LOC);

   Log(LOG_UserFunction__fix)   fun->print(CERR);
   return fun;
}
//=============================================================================