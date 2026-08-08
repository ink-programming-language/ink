// Translated from solution.cpp.

var res = cpp_array(2, 15);

var commands = cpp_array(500005);

var num = cpp_array(500005);

var Xor: dynamic;

var Or: dynamic;

var And: dynamic;

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var st = 1;
  var j2: dynamic;
  var b: dynamic;
  var b2: dynamic;
  var BB: dynamic;
  read(n);
  {
    i = 0;
    while ((i < n))
    {
      read(commands[i], num[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= 9))
    {
      {
        j2 = 0;
        while ((j2 <= 1))
        {
          b = j2;
          {
            j = 0;
            while ((j < n))
            {
              b2 = (((num[j] >> i)) & 1);
              if ((commands[j] == cpp_char("|")))
              {
                b = (b2 | b);
              } else
              {
                if ((commands[j] == cpp_char("&")))
                {
                  b = (b2 & b);
                } else
                {
                  b = (b2 ^ b);
                }
              }
              j += 1;
            }
          }
          if ((j2 == 0))
          {
            BB = b;
          } else
          {
            if (((BB == 0) && (b == 0)))
            {
              Or += st;
              Xor += st;
            }
            if (((BB == 1) && (b == 0)))
            {
              Xor += st;
            }
            if (((BB == 1) && (b == 1)))
            {
              Or += st;
            }
          }
          j2 += 1;
        }
      }
      st *= 2;
      i += 1;
    }
  }
  write("2", "\n");
  write("| ", Or, "\n");
  write("^ ", Xor, "\n");
  return 0;
}
