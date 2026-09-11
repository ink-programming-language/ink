// Translated from solution.cpp.

var MAX: dynamic = 200000;

var INF: dynamic = 100000000;

var MOD: dynamic = 1000000007;

var EPS: dynamic = 1E-7;

var IT: dynamic = 10024;

var r: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var k: dynamic = cpp_uninitialized();
  read(k);
  {
    var i: dynamic = (0);
    while ((i < k))
    {
      var x1: dynamic = cpp_uninitialized();
      var y1: dynamic = cpp_uninitialized();
      var x2: dynamic = cpp_uninitialized();
      var y2: dynamic = cpp_uninitialized();
      scanf("%d%d%d%d", (&x1), (&y1), (&x2), (&y2));
      if ((x1 == x2))
      {
        r[x1].push_back(make_pair(min(y1, y2), max(y1, y2)));
      } else
      {
        c[y1].push_back(make_pair(min(x1, x2), max(x1, x2)));
      }
      i += 1;
    }
  }
  var dr: dynamic = ((n - 1) - r.size());
  var dc: dynamic = ((m - 1) - c.size());
  var res: dynamic = 0;
  if ((dr & 1))
  {
    res ^= m;
  }
  if ((dc & 1))
  {
    res ^= n;
  }
  {
    var it: dynamic = r.begin();
    while ((it != r.end()))
    {
      var cnt: dynamic = 0;
      sort(it->second.begin(), it->second.end());
      cnt += it->second[0].first;
      var rigth: dynamic = it->second[0].second;
      {
        var i: dynamic = (1);
        while ((i < it->second.size()))
        {
          cnt += max(0, (it->second[i].first - rigth));
          rigth = max(rigth, it->second[i].second);
          i += 1;
        }
      }
      cnt += (m - rigth);
      R[it->first] = cnt;
      res ^= cnt;
      it += 1;
    }
  }
  {
    var it: dynamic = c.begin();
    while ((it != c.end()))
    {
      var cnt: dynamic = 0;
      sort(it->second.begin(), it->second.end());
      cnt += it->second[0].first;
      var rigth: dynamic = it->second[0].second;
      {
        var i: dynamic = (1);
        while ((i < it->second.size()))
        {
          cnt += max(0, (it->second[i].first - rigth));
          rigth = max(rigth, it->second[i].second);
          i += 1;
        }
      }
      cnt += (n - rigth);
      C[it->first] = cnt;
      res ^= cnt;
      it += 1;
    }
  }
  if ((res == 0))
  {
    write("SECOND\n");
    return 0;
  } else
  {
    write("FIRST\n");
  }
  if ((dr && (((res ^ m)) <= m)))
  {
    var cut: dynamic = (m - ((res ^ m)));
    var X: dynamic = cpp_uninitialized();
    {
      var i: dynamic = (1);
      while ((i < 100007))
      {
        if ((!R.count(i)))
        {
          X = i;
          break;
        }
        i += 1;
      }
    }
    write(X, cpp_char(" "), 0, cpp_char(" "), X, cpp_char(" "), cut, "\n");
    return 0;
  }
  if ((dc && (((res ^ n)) <= n)))
  {
    var cut: dynamic = (n - ((res ^ n)));
    var X: dynamic = cpp_uninitialized();
    {
      var i: dynamic = (1);
      while ((i < 100007))
      {
        if ((!C.count(i)))
        {
          X = i;
          break;
        }
        i += 1;
      }
    }
    write(0, cpp_char(" "), X, cpp_char(" "), cut, cpp_char(" "), X, "\n");
    return 0;
  }
  {
    var it: dynamic = R.begin();
    while ((it != R.end()))
    {
      if ((((res ^ it->second)) <= it->second))
      {
        var cut: dynamic = (it->second - ((res ^ it->second)));
        var x: dynamic = it->first;
        var temp: dynamic = r[x];
        var cnt: dynamic = 0;
        cnt += temp[0].first;
        if ((cut <= temp[0].first))
        {
          write(x, cpp_char(" "), 0, cpp_char(" "), x, cpp_char(" "), cut, "\n");
          return 0;
        }
        var rigth: dynamic = temp[0].second;
        {
          var i: dynamic = (1);
          while ((i < temp.size()))
          {
            var add: dynamic = max(0, (temp[i].first - rigth));
            if (((cnt + add) >= cut))
            {
              write(x, cpp_char(" "), 0, cpp_char(" "), x, cpp_char(" "), ((rigth + cut) - cnt), "\n");
              return 0;
            }
            cnt += add;
            rigth = max(rigth, temp[i].second);
            i += 1;
          }
        }
        write(x, cpp_char(" "), 0, cpp_char(" "), x, cpp_char(" "), ((rigth + cut) - cnt), "\n");
        return 0;
      }
      it += 1;
    }
  }
  {
    var it: dynamic = C.begin();
    while ((it != C.end()))
    {
      if ((((res ^ it->second)) <= it->second))
      {
        var cut: dynamic = (it->second - ((res ^ it->second)));
        var x: dynamic = it->first;
        var temp: dynamic = c[x];
        var cnt: dynamic = 0;
        if ((cut <= temp[0].first))
        {
          write(0, cpp_char(" "), x, cpp_char(" "), cut, cpp_char(" "), x, "\n");
          return 0;
        }
        cnt += temp[0].first;
        var rigth: dynamic = temp[0].second;
        {
          var i: dynamic = (1);
          while ((i < temp.size()))
          {
            var add: dynamic = max(0, (temp[i].first - rigth));
            if (((cnt + add) >= cut))
            {
              write(0, cpp_char(" "), x, cpp_char(" "), ((rigth + cut) - cnt), cpp_char(" "), x, "\n");
              return 0;
            }
            cnt += add;
            rigth = max(rigth, temp[i].second);
            i += 1;
          }
        }
        write(0, cpp_char(" "), x, cpp_char(" "), ((rigth + cut) - cnt), cpp_char(" "), x, "\n");
        return 0;
      }
      it += 1;
    }
  }
}
