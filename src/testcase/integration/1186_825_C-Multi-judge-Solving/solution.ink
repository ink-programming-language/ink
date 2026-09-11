// Translated from solution.cpp.

func preprocess(argument_0: dynamic) -> dynamic
{
  return;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.precision(20);
  preprocess();
  var n: dynamic = cpp_uninitialized();
  read(n);
  var k: dynamic = cpp_uninitialized();
  read(k);
  {
    var i: dynamic = (0);
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  var cnt: dynamic = 0;
  {
    var i: dynamic = (0);
    while ((i < n))
    {
      if (((k * 2) >= a[i]))
      {
        k = max(k, cpp_cast(a[i]));
      } else
      {
        {
          var j: dynamic = (1);
          while ((j < 34))
          {
            if (((k * 2) < a[i]))
            {
              k = (k * 2);
              cnt += 1;
            } else
            {
              break;
            }
            j += 1;
          }
        }
        k = max(k, cpp_cast(a[i]));
      }
      i += 1;
    }
  }
  write(cnt, "\n");
}
