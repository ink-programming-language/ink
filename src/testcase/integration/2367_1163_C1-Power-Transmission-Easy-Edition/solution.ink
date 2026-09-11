// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var point: dynamic = cpp_uninitialized();
  var i: dynamic = 0;
  while ((i < n))
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(x, y);
    point.push_back([x, y]);
    i += 1;
  }
  var m3: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          if ((point[i].first == point[j].first))
          {
            var c: dynamic = point[i].first;
            p.insert(c);
          } else
          {
            var m: dynamic = (cpp_cast(((point[i].second - point[j].second))) / ((point[i].first - point[j].first)));
            var c: dynamic = (cpp_cast(point[i].second) - ((m * point[i].first)));
            m3[m].insert(c);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  n = p.size();
  for (var it: dynamic in m3)
  {
    n += it.second.size();
  }
  var ans: dynamic = (((n * ((n - 1)))) / 2);
  ans -= (((((p.size() - 1)) * p.size())) / 2);
  for (var it: dynamic in m3)
  {
    ans -= (((((it.second.size() - 1)) * it.second.size())) / 2);
  }
  write(ans);
}
