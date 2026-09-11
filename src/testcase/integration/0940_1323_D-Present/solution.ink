// Translated from solution.cpp.

var N: dynamic = (4e5 + 5);

var a: dynamic = cpp_array(N);

var t: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var k: dynamic = 0;
    while ((k <= 24))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          t[i] = (a[i] % ((1 << ((k + 1)))));
          i += 1;
        }
      }
      sort(t, (t + n));
      var cnt: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < (n - 1)))
        {
          cnt += ((lower_bound(((t + i) + 1), (t + n), (((1 << ((k + 1)))) - t[i])) - lower_bound(((t + i) + 1), (t + n), (((1 << k)) - t[i]))));
          cnt += (n - ((lower_bound(((t + i) + 1), (t + n), ((((1 << k)) + ((1 << ((k + 1))))) - t[i])) - t)));
          i += 1;
        }
      }
      ans |= (((1 << k)) * ((cnt % 2)));
      k += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
