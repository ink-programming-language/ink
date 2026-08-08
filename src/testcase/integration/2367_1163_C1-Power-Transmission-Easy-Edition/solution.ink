// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var point: dynamic;
  var i = 0;
  while ((i < n))
  {
    var x: dynamic;
    var y: dynamic;
    read(x, y);
    point.push_back([x, y]);
    i += 1;
  }
  var m3: dynamic;
  var p: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          if ((point[i].first == point[j].first))
          {
            var c = point[i].first;
            p.insert(c);
          } else
          {
            var m = (cpp_cast(((point[i].second - point[j].second))) / ((point[i].first - point[j].first)));
            var c = (cpp_cast(point[i].second) - ((m * point[i].first)));
            m3[m].insert(c);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  n = p.size();
  for (var it in m3)
  {
    n += it.second.size();
  }
  var ans = (((n * ((n - 1)))) / 2);
  ans -= (((((p.size() - 1)) * p.size())) / 2);
  for (var it in m3)
  {
    ans -= (((((it.second.size() - 1)) * it.second.size())) / 2);
  }
  write(ans);
}
