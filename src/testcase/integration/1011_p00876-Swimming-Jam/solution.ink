// Translated from solution.cpp.

var N: dynamic = 50;

var EPS: dynamic = 1e-8;

var n: dynamic = cpp_uninitialized();

var data: dynamic = cpp_uninitialized();

func equals(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) < EPS);
}

func solve() -> dynamic
{
  var dist: dynamic = cpp_array(N);
  var round: dynamic = cpp_array(N);
  var res: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      dist[i] = make_pair(0.0, 0.0);
      round[i] = 1;
      i += 1;
    }
  }
  {
    res = 0;
    while (true)
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if (data[i].second)
          {
            cpp_goto("goto CONT;");
          }
          i += 1;
        }
      }
      break;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((data[i].second == 0))
          {
            i += 1;
            continue;
          }
          dist[i] = make_pair(dist[i].second, (dist[i].second + data[i].first));
          {
            var j: dynamic = 0;
            while ((j < i))
            {
              if ((data[j].second == 0))
              {
                j += 1;
                continue;
              }
              if ((round[i] != round[j]))
              {
                j += 1;
                continue;
              }
              if ((equals(dist[i].first, 0.0) && equals(dist[j].first, 0.0)))
              {
                j += 1;
                continue;
              }
              if (((!equals(dist[j].first, dist[i].first)) && (dist[j].first < dist[i].first)))
              {
                j += 1;
                continue;
              }
              if (((!equals(dist[i].first, dist[j].second)) && (dist[i].first < dist[j].second)))
              {
                dist[i].second = min(dist[i].second, dist[j].second);
              }
              j += 1;
            }
          }
          dist[i].second = min(dist[i].second, 1.0);
          if (equals(dist[i].second, 1.0))
          {
            dist[i].second = 0.0;
            if ((round[i] == 0))
            {
              data[i].second -= 1;
            }
            round[i] = (1 - round[i]);
          }
          i += 1;
        }
      }
      res += 1;
    }
  }
  return res;
}

func main() -> dynamic
{
  while (((cin >> n) && n))
  {
    data.clear();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var a: dynamic = cpp_uninitialized();
        var b: dynamic = cpp_uninitialized();
        read(a, b);
        data.push_back(make_pair((1.0 / a), b));
        i += 1;
      }
    }
    sort(data.begin(), data.end());
    write(solve(), "\n");
  }
}
