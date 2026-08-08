// Translated from solution.cpp.

var A = cpp_array(100001);

var op: dynamic;

var cnt: dynamic;

var n: dynamic;

var at: dynamic;

func solve()
{
  while ((cnt < op))
  {
    if ((at == (n - 1)))
    {
      break;
    } else
    {
      if (((A[at] == cpp_char("4")) && (A[(at + 1)] == cpp_char("7"))))
      {
        if (((at % 2) == 0))
        {
          if ((at == (n - 2)))
          {
            A[(at + 1)] = cpp_char("4");
            cnt += 1;
            at += 1;
          } else if ((A[(at + 2)] != 7))
          {
            A[(at + 1)] = cpp_char("4");
            cnt += 1;
            at += 1;
          } else
          {
            if (((((op - cnt)) % 2) == 1))
            {
              A[(at + 1)] = cpp_char("4");
            }
            cnt = (op + 1);
          }
        } else
        {
          if ((at > 0))
          {
            if ((A[(at - 1)] == cpp_char("4")))
            {
              if (((((op - cnt)) % 2) == 1))
              {
                A[at] = cpp_char("7");
              }
              cnt = (op + 1);
            } else
            {
              cnt += 1;
              A[at] = cpp_char("7");
              at += 1;
            }
          }
        }
      } else
      {
        at += 1;
      }
    }
  }
}

func main()
{
  scanf("%d%d", (&n), (&op));
  scanf("%s", A);
  if ((n > 1))
  {
    at = 0;
    solve();
  }
  printf("%s\n", A);
  return 0;
}
