// Translated from solution.cpp.

var ax: dynamic = cpp_array(200005);

var dp: dynamic = cpp_array(200005);

var len: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%s", ax);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var now: dynamic = 0;
  len = strlen(ax);
  m[0] = len;
  {
    i = (len - 1);
    while ((i >= 0))
    {
      now ^= ((1 << ((ax[i] - cpp_char("a")))));
      dp[i] = (len - i);
      if (m.count(now))
      {
        dp[i] = min(dp[i], (dp[m[now]] + 1));
      }
      {
        j = 0;
        while ((j < 26))
        {
          a = (now ^ ((1 << j)));
          if ((!m.count(a)))
          {
            j += 1;
            continue;
          }
          dp[i] = min(dp[i], (dp[m[a]] + 1));
          j += 1;
        }
      }
      if ((!m.count(now)))
      {
        m[now] = i;
      } else if ((dp[i] < dp[m[now]]))
      {
        m[now] = i;
      }
      i -= 1;
    }
  }
  printf("%d", dp[0]);
  return 0;
}
