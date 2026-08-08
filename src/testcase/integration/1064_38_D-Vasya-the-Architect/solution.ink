// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  solve();
  return 0;
}

var rects = cpp_array(100);

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(rects[i].first.first, rects[i].first.second, rects[i].second.first, rects[i].second.second);
      if ((rects[i].first.first > rects[i].second.first))
      {
        swap(rects[i].first.first, rects[i].second.first);
      }
      if ((rects[i].first.second > rects[i].second.second))
      {
        swap(rects[i].first.second, rects[i].second.second);
      }
      {
        var j = 0;
        while ((j < i))
        {
          var cx = 0;
          var cy = 0;
          var m = 0;
          {
            var k = (j + 1);
            while ((k <= i))
            {
              var l = fabs((rects[k].first.first - rects[k].second.first));
              var lll = ((l * l) * l);
              m += lll;
              var x = (((rects[k].first.first + rects[k].second.first)) / 2.0);
              var y = (((rects[k].first.second + rects[k].second.second)) / 2.0);
              cx += (lll * x);
              cy += (lll * y);
              k += 1;
            }
          }
          cx /= m;
          cy /= m;
          if (((((rects[j].first.first > cx) || (rects[j].first.second > cy)) || (rects[j].second.first < cx)) || (rects[j].second.second < cy)))
          {
            write(i);
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(n);
}
