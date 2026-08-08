// Translated from solution.cpp.

var N = 100002;

var a = cpp_array(N);

var d = cpp_array(N);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    i = 1;
    while ((i <= n))
    {
      var cnt = 0;
      {
        j = 2;
        while (((j * j) <= a[i]))
        {
          if (((a[i] % j) == 0))
          {
            cnt = max(cnt, d[j]);
            cnt = max(cnt, d[(a[i] / j)]);
          }
          j += 1;
        }
      }
      {
        j = 2;
        while (((j * j) <= a[i]))
        {
          if (((a[i] % j) == 0))
          {
            d[j] = max(d[j], (cnt + 1));
            d[(a[i] / j)] = max(d[(a[i] / j)], (cnt + 1));
          }
          j += 1;
        }
      }
      d[a[i]] = max(d[a[i]], (cnt + 1));
      ans = max((cnt + 1), ans);
      i += 1;
    }
  }
  write(ans);
}
