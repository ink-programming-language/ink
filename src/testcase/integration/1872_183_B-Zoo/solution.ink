// Translated from solution.cpp.

var f: dynamic = cpp_array(1000001);

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
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
      var x1: dynamic = v[i].first;
      var y1: dynamic = v[i].second;
      {
        j = (i + 1);
        while ((j < m))
        {
          var x2: dynamic = v[j].first;
          var y2: dynamic = v[j].second;
          var w: dynamic = ((x1 * y2) - (x2 * y1));
          if (((((y1 - y2)) != 0) && ((w % ((y2 - y1))) == 0)))
          {
            var id: dynamic = (w / ((y2 - y1)));
            if (((id > 0) && (id <= n)))
            {
              var sum: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    i = 1;
    while ((i <= n))
    {
      ans +=  (f[i]) ? f[i] : 1;
      i += 1;
    }
  }
  write(ans, "\n");
}
