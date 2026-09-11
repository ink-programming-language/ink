// Translated from solution.cpp.

var dp: dynamic = cpp_array(1000000);

var arr: dynamic = cpp_array(1000000);

var arr2: dynamic = cpp_array(300);

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var maxn: dynamic = 1;

var mx: dynamic = 1;

func main() -> dynamic
{
  read(n);
  read(t);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      arr2[arr[i]] += 1;
      mx = max(mx, arr2[arr[i]]);
      dp[i] = 1;
      i += 1;
    }
  }
  var k: dynamic = min(n, t);
  {
    var i: dynamic = n;
    while ((i < (n * k)))
    {
      arr[i] = arr[(i - n)];
      dp[i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n * k)))
    {
      {
        var j: dynamic = 0;
        while ((j < i))
        {
          if ((arr[i] >= arr[j]))
          {
            dp[i] = max(dp[i], (dp[j] + 1));
            maxn = max(maxn, dp[i]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((t > n))
  {
    maxn += (mx * ((t - n)));
  }
  write(maxn, "\n");
  return 0;
}
