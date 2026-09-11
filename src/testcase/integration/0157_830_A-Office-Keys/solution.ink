// Translated from solution.cpp.

class A830
{
  func cost(a: dynamic, b: dynamic, c: dynamic) -> dynamic
  {
      return (abs((a - b)) + abs((b - c)));
    }
  func solve(in_cpp: dynamic, out: dynamic) -> dynamic
  {
      var n: dynamic = cpp_uninitialized();
      var k: dynamic = cpp_uninitialized();
      var p: dynamic = cpp_uninitialized();
      (((in_cpp >> n) >> k) >> p);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          (in_cpp >> a[i]);
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < k))
        {
          (in_cpp >> b[i]);
          i += 1;
        }
      }
      sort(begin(a), end(a));
      sort(begin(b), end(b));
      var low: dynamic = 0;
      var high: dynamic = 1e11;
      var ans: dynamic = high;
      while ((low <= high))
      {
        var mid: dynamic = (((low + high)) / 2);
        var ok1: dynamic = true;
        {
          var i: dynamic = 0;
          var j: dynamic = 0;
          while ((i < n))
          {
            while (((j < k) && (cost(a[i], b[j], p) > mid)))
            {
              j += 1;
            }
            if ((j == k))
            {
              ok1 = false;
              break;
            }
            j += 1;
            i += 1;
          }
        }
        var ok2: dynamic = true;
        {
          var i: dynamic = (n - 1);
          var j: dynamic = (k - 1);
          while ((i >= 0))
          {
            while (((j >= 0) && (cost(a[i], b[j], p) > mid)))
            {
              j -= 1;
            }
            if ((j == -1))
            {
              ok2 = false;
              break;
            }
            j -= 1;
            i -= 1;
          }
        }
        if ((ok1 || ok2))
        {
          high = (mid - 1);
          ans = mid;
        } else
        {
          low = (mid + 1);
        }
      }
      ((out << ans) << "\n");
    }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var solver: dynamic = cpp_uninitialized();
  var in_cpp: dynamic = cpp_uninitialized();
  var out: dynamic = cpp_uninitialized();
  solver.solve(in_cpp, out);
  return 0;
}
