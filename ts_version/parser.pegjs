top_level = _ a:(_ e:(action / fn_action) _ { return e })* _ { return a }

action "action" = c_code / key_action

key_action "call" = !'fn' name:keyword _ args:(_ e:expr _ { return e })* _ ';' { return ['call', name, args] }
fn_action "function" = 'fn' _ name:keyword _ body:block _ ';' { return ['function', name, body] }

nosemi_action = nosemi_key_action / nosemi_c_code
nosemi_key_action "call" = !'fn' name:keyword _ args:(_ e:expr _ { return e })* { return ['call', name, args] }
nosemi_c_code "c code" = 'c{' _ code:([^\n}]*) _ '}' { return ['c', code.join('').trim()] }

expr "expression" = c_code / string / number / keyword / block / char

c_code "c code" = 'c{' _ code:([^\n}]*) _ '}' _ ';' { return ['c', code.join('').trim()] }
block "block" = '[' _ actions:(_ e:action _ { return e })* _ final:nosemi_action? _ ']' { return ['block', [...actions, final].filter(e => e)] }
number "number" = [0-9]+ { return ['number', parseInt(text())] }
string "string" = '"' txt:([^\n"]*) '"' { return ['string', txt.join('')] }
char "character" = '\'' c:[^\n'] '\'' { return ['char', c] }
keyword "keyword" = [a-zA-Z_][a-zA-Z0-9_]* { return ['keyword', text()] }

_ "whitespace" = (comment / [ \t\n])* { return undefined }
comment "comment" = '#' [ \t]* [^\n]* [ \t]* '\n'? { return undefined }