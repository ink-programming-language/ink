// Translated from solution.cpp.

func max(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x > y)) ? x : y;
}

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(1002000);

var dp: dynamic = cpp_array(1002000);

var ma: dynamic = cpp_array(1002000);

var sec: dynamic = cpp_array(1002000);

func Update(x: dynamic) -> dynamic
{
  var o: dynamic = dp[x];
  var pos: dynamic = 0;
  if ((ma[f[x]] == dp[x]))
  {
    pos = 1;
  }
  dp[x] = max(ma[x], (sec[x] + 1));
  if ((pos == 0))
  {
    if ((ma[f[x]] <= dp[x]))
    {
      sec[f[x]] = ma[f[x]];
      ma[f[x]] = dp[x];
    } else if ((sec[f[x]] <= dp[x]))
    {
      sec[f[x]] = dp[x];
    }
  } else
  {
    ma[f[x]] = dp[x];
  }
  if ((dp[x] == o))
  {
    return 0;
  } else
  {
    return 1;
  }
}

func gi(x: dynamic) -> dynamic
{
  var ch: dynamic = getchar();
  x = 0;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - 48);
    ch = getchar();
  }
}

func pi(x: dynamic) -> dynamic
{
  if ((x > 9))
  {
    pi((x / 10));
  }
  putchar(((x % 10) + 48));
}

func main() -> dynamic
{
  gi(n);
  n += 1;
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      gi(f[i]);
      dp[i] = 1;
      if ((dp[i] > ma[f[i]]))
      {
        sec[f[i]] = ma[f[i]];
        ma[f[i]] = dp[i];
      } else if ((dp[i] > sec[f[i]]))
      {
        sec[f[i]] = dp[i];
      }
      Update(i);
      {
        var j: dynamic = f[i];
        while (j)
        {
          if ((!Update(j)))
          {
            break;
          }
          j = f[j];
        }
      }
      pi(ma[1]);
      putchar(cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
