// Translated from solution.cpp.

var N: dynamic = 100002;

var a: dynamic = cpp_array(N);

var d: dynamic = cpp_array(N);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    i = 1;
    while ((i <= n))
    {
      var cnt: dynamic = 0;
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
