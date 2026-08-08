// Translated from solution.cpp.

var MAX = 200000;

var INF = 100000000;

var MOD = 1000000007;

var EPS = 1E-7;

var IT = 10024;

var r: dynamic;

var c: dynamic;

var R: dynamic;

var C: dynamic;

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var k: dynamic;
  read(k);
  {
    var i = (0);
    while ((i < k))
    {
      var x1: dynamic;
      var y1: dynamic;
      var x2: dynamic;
      var y2: dynamic;
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
  var dr = ((n - 1) - r.size());
  var dc = ((m - 1) - c.size());
  var res = 0;
  if ((dr & 1))
  {
    res ^= m;
  }
  if ((dc & 1))
  {
    res ^= n;
  }
  {
    var it = r.begin();
    while ((it != r.end()))
    {
      var cnt = 0;
      sort(it->second.begin(), it->second.end());
      cnt += it->second[0].first;
      var rigth = it->second[0].second;
      {
        var i = (1);
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
    var it = c.begin();
    while ((it != c.end()))
    {
      var cnt = 0;
      sort(it->second.begin(), it->second.end());
      cnt += it->second[0].first;
      var rigth = it->second[0].second;
      {
        var i = (1);
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
    var cut = (m - ((res ^ m)));
    var X: dynamic;
    {
      var i = (1);
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
    var cut = (n - ((res ^ n)));
    var X: dynamic;
    {
      var i = (1);
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
    var it = R.begin();
    while ((it != R.end()))
    {
      if ((((res ^ it->second)) <= it->second))
      {
        var cut = (it->second - ((res ^ it->second)));
        var x = it->first;
        var temp = r[x];
        var cnt = 0;
        cnt += temp[0].first;
        if ((cut <= temp[0].first))
        {
          write(x, cpp_char(" "), 0, cpp_char(" "), x, cpp_char(" "), cut, "\n");
          return 0;
        }
        var rigth = temp[0].second;
        {
          var i = (1);
          while ((i < temp.size()))
          {
            var add = max(0, (temp[i].first - rigth));
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
    var it = C.begin();
    while ((it != C.end()))
    {
      if ((((res ^ it->second)) <= it->second))
      {
        var cut = (it->second - ((res ^ it->second)));
        var x = it->first;
        var temp = c[x];
        var cnt = 0;
        if ((cut <= temp[0].first))
        {
          write(0, cpp_char(" "), x, cpp_char(" "), cut, cpp_char(" "), x, "\n");
          return 0;
        }
        cnt += temp[0].first;
        var rigth = temp[0].second;
        {
          var i = (1);
          while ((i < temp.size()))
          {
            var add = max(0, (temp[i].first - rigth));
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
