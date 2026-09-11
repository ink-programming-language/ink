// Translated from solution.cpp.

func input() -> dynamic
{
  var ret: dynamic = 0;
  var isN: dynamic = 0;
  var c: dynamic = getchar();
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
  return  (isN) ? (-ret) : ret;
}

func output(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var len: dynamic = 0;
  var data: dynamic = cpp_array(10);
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

var MAXN: dynamic = 1010;

var s: dynamic = cpp_array(MAXN);

var k: dynamic = cpp_uninitialized();

func in_cpp() -> dynamic
{
  scanf("%s%d", s, (&k));
}

func ok(st: dynamic, ans: dynamic) -> dynamic
{
  var nxt: dynamic = (st + ans);
  var j: dynamic = nxt;
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

func work() -> dynamic
{
  var l: dynamic = strlen(s);
  if ((k >= l))
  {
    printf("%d\n", ((((k + l)) / 2) * 2));
  } else
  {
    var ans: dynamic = cpp_uninitialized();
    var tag: dynamic = 0;
    {
      ans = (((k + l)) / 2);
      while ((ans >= 1))
      {
        {
          var i: dynamic = 0;
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

func main() -> dynamic
{
  in_cpp();
  work();
  return 0;
}
