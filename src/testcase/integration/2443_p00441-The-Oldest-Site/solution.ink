// Translated from solution.cpp.

var p: dynamic = cpp_array(3000);

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while (((cin >> n) && n))
  {
    ans = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d %d", (&p[i].first), (&p[i].second));
        i += 1;
      }
    }
    sort(p, (p + n));
    {
      var i: dynamic = 0;
      while (((i + 1) < n))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            var a: dynamic = p[i];
            var b: dynamic = p[j];
            var c: dynamic = cpp_uninitialized();
            var d: dynamic = cpp_uninitialized();
            c.first = ((a.first + a.second) - b.second);
            c.second = ((a.second + b.first) - a.first);
            d.first = ((b.first + a.second) - b.second);
            d.second = ((b.second + b.first) - a.first);
            if ((binary_search(p, (p + n), c) == false))
            {
              j += 1;
              continue;
            }
            if ((binary_search(p, (p + n), d) == false))
            {
              j += 1;
              continue;
            }
            var d1: dynamic = ((a.first - b.first));
            var d2: dynamic = ((a.second - b.second));
            var d3: dynamic = ((d1 * d1) + (d2 * d2));
            ans = max(ans, d3);
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
