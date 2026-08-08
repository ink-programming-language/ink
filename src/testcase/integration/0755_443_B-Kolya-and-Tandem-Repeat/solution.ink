// Translated from solution.cpp.

func input()
{
  var ret = 0;
  var isN = 0;
  var c = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      isN = 1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    ret = (((ret * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return if (isN) (-ret) else ret;
}

func output(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var len = 0;
  var data = cpp_array(10);
  while (x)
  {
    data[cpp_update(len, "++")] = (x % 10);
    x /= 10;
  }
  if ((!len))
  {
    data[cpp_update(len, "++")] = 0;
  }
  while (cpp_update(len, "--"))
  {
    putchar((data[len] + 48));
  }
  putchar(cpp_char("\n"));
}

var MAXN = 1010;

var s = cpp_array(MAXN);

var k: dynamic;

func in_cpp()
{
  scanf("%s%d", s, (&k));
}

func ok(st: dynamic, ans: dynamic)
{
  var nxt = (st + ans);
  var j = nxt;
  while (((st < j) && (nxt < strlen(s))))
  {
    if ((s[st] != s[nxt]))
    {
      return 0;
    }
    st += 1;
    nxt += 1;
  }
  return 1;
}

func work()
{
  var l = strlen(s);
  if ((k >= l))
  {
    printf("%d\n", ((((k + l)) / 2) * 2));
  } else
  {
    var ans: dynamic;
    var tag = 0;
    {
      ans = (((k + l)) / 2);
      while ((ans >= 1))
      {
        {
          var i = 0;
          while ((((i + (ans * 2)) - 1) < ((k + l))))
          {
            if (ok(i, ans))
            {
              tag = 1;
              break;
            }
            i += 1;
          }
        }
        if (tag)
        {
          break;
        }
        ans -= 1;
      }
    }
    printf("%d\n", (ans * 2));
  }
}

func main()
{
  in_cpp();
  work();
  return 0;
}
