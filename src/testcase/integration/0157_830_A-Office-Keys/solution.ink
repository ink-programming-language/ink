// Translated from solution.cpp.

class A830
{
  func cost(a: dynamic, b: dynamic, c: dynamic)
  {
      return (abs((a - b)) + abs((b - c)));
    }
  func solve(in_cpp: dynamic, out: dynamic)
  {
      var n: dynamic;
      var k: dynamic;
      var p: dynamic;
      (((in_cpp >> n) >> k) >> p);
      {
        var i = 0;
        while ((i < n))
        {
          (in_cpp >> a[i]);
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < k))
        {
          (in_cpp >> b[i]);
          i += 1;
        }
      }
      sort(begin(a), end(a));
      sort(begin(b), end(b));
      var low = 0;
      var high = 1e11;
      var ans = high;
      while ((low <= high))
      {
        var mid = (((low + high)) / 2);
        var ok1 = true;
        {
          var i = 0;
          var j = 0;
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
        var ok2 = true;
        {
          var i = (n - 1);
          var j = (k - 1);
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var solver: dynamic;
  var in_cpp: dynamic;
  var out: dynamic;
  solver.solve(in_cpp, out);
  return 0;
}
