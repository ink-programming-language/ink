// Translated from solution.cpp.

var dp = cpp_array(1000000);

var arr = cpp_array(1000000);

var arr2 = cpp_array(300);

var n: dynamic;

var t: dynamic;

var maxn = 1;

var mx = 1;

func main()
{
  read(n);
  read(t);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      arr2[arr[i]] += 1;
      mx = max(mx, arr2[arr[i]]);
      dp[i] = 1;
      i += 1;
    }
  }
  var k = min(n, t);
  {
    var i = n;
    while ((i < (n * k)))
    {
      arr[i] = arr[(i - n)];
      dp[i] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (n * k)))
    {
      {
        var j = 0;
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
