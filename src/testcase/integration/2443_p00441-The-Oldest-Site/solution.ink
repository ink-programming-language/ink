// Translated from solution.cpp.

var p = cpp_array(3000);

var n: dynamic;

var ans: dynamic;

func main()
{
  while (((cin >> n) && n))
  {
    ans = 0;
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%d %d", (&p[i].first), (&p[i].second));
        i += 1;
      }
    }
    sort(p, (p + n));
    {
      var i = 0;
      while (((i + 1) < n))
      {
        {
          var j = (i + 1);
          while ((j < n))
          {
            var a = p[i];
            var b = p[j];
            var c: dynamic;
            var d: dynamic;
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
            var d1 = ((a.first - b.first));
            var d2 = ((a.second - b.second));
            var d3 = ((d1 * d1) + (d2 * d2));
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
