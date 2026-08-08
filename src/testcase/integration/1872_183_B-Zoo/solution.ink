// Translated from solution.cpp.

var f = cpp_array(1000001);

var v: dynamic;

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  read(n, m);
  {
    i = 0;
    while ((i < m))
    {
      read(j, k);
      v.push_back(make_pair(j, k));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      var x1 = v[i].first;
      var y1 = v[i].second;
      {
        j = (i + 1);
        while ((j < m))
        {
          var x2 = v[j].first;
          var y2 = v[j].second;
          var w = ((x1 * y2) - (x2 * y1));
          if (((((y1 - y2)) != 0) && ((w % ((y2 - y1))) == 0)))
          {
            var id = (w / ((y2 - y1)));
            if (((id > 0) && (id <= n)))
            {
              var sum = 0;
              {
                k = 0;
                while ((k < m))
                {
                  if (((((y1 - y2)) * ((x1 - v[k].first))) == (((x1 - x2)) * ((y1 - v[k].second)))))
                  {
                    sum += 1;
                  }
                  k += 1;
                }
              }
              f[id] = max(f[id], sum);
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    i = 1;
    while ((i <= n))
    {
      ans += if (f[i]) f[i] else 1;
      i += 1;
    }
  }
  write(ans, "\n");
}
