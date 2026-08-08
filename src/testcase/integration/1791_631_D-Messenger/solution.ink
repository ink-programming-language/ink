// Translated from solution.cpp.

var INF = int_cpp(1e9);

var INFll = ((1 * INF) * INF);

var ldINF = 1e+018;

var EPS = 0.000000001;

var N = 100001;

func operator_shift_left(out: dynamic, a: dynamic)
{
  (((out << a.first) << " ") << a.second);
  return out;
}

func operator_shift_right(in_cpp: dynamic, a: dynamic)
{
  ((in_cpp >> a.first) >> a.second);
  return in_cpp;
}

func operator_shift_left(out: dynamic, a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      ((out << a[i]) << endl);
      i += 1;
    }
  }
  return out;
}

func operator_shift_right(in_cpp: dynamic, a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      (in_cpp >> a[i]);
      i += 1;
    }
  }
  return in_cpp;
}

func operator_shift_left(out: dynamic, a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      if ((i == (a.size() - 1)))
      {
        (out << a[i]);
      } else
      {
        ((out << a[i]) << " ");
      }
      i += 1;
    }
  }
  return out;
}

func compress(a: dynamic)
{
  var res: dynamic;
  res.push_back(a[0]);
  {
    var i = 1;
    while ((i < a.size()))
    {
      if ((a[i].second == a[(i - 1)].second))
      {
        res.back().first += a[i].first;
      } else
      {
        res.push_back(a[i]);
      }
      i += 1;
    }
  }
  a = res;
}

func main()
{
  ios_base.sync_with_stdio(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var z = n;
  {
    var i = 0;
    while ((i < n))
    {
      var t: dynamic;
      read(t);
      var num = 0;
      var x = 0;
      {
        var j = 0;
        while ((j < t.size()))
        {
          if (((t[j] < cpp_char("0")) || (t[j] > cpp_char("9"))))
          {
            num = j;
            break;
          } else
          {
            x = ((x * 10) + ((t[j] - cpp_char("0"))));
          }
          j += 1;
        }
      }
      var y = "";
      y = t.substr((num + 1), ((t.size() - num) - 1));
      a[i] = make_pair(x, y);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var t: dynamic;
      read(t);
      var num = 0;
      var x = 0;
      {
        var j = 0;
        while ((j < t.size()))
        {
          if (((t[j] < cpp_char("0")) || (t[j] > cpp_char("9"))))
          {
            num = j;
            break;
          } else
          {
            x = ((x * 10) + ((t[j] - cpp_char("0"))));
          }
          j += 1;
        }
      }
      var y = "";
      y = t.substr((num + 1), ((t.size() - num) - 1));
      b[i] = make_pair(x, y);
      i += 1;
    }
  }
  compress(a);
  compress(b);
  n = a.size();
  m = b.size();
  if ((m == 1))
  {
    var ans = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if (((a[i].second == b[0].second) && (a[i].first >= b[0].first)))
        {
          ans += (((a[i].first - b[0].first) + 1));
        }
        i += 1;
      }
    }
    write(ans, "\n");
    return 0;
  }
  if ((m == 2))
  {
    var ans = 0;
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if (((((a[i].second == b[0].second) && (a[(i + 1)].second == b[1].second)) && (a[i].first >= b[0].first)) && (a[(i + 1)].first >= b[1].first)))
        {
          ans += 1;
        }
        i += 1;
      }
    }
    write(ans, "\n");
    return 0;
  }
  var s: dynamic;
  {
    var i = 1;
    while ((i < (m - 1)))
    {
      s.push_back(b[i]);
      i += 1;
    }
  }
  s.push_back(make_pair(-1, "#"));
  {
    var i = 0;
    while ((i < a.size()))
    {
      s.push_back(a[i]);
      i += 1;
    }
  }
  var p = cpp_construct(s.size(), 0);
  {
    var i = 1;
    while ((i < s.size()))
    {
      var j = p[(i - 1)];
      while (((j > 0) && (s[i] != s[j])))
      {
        j = p[(j - 1)];
      }
      if ((s[i] == s[j]))
      {
        j += 1;
      }
      p[i] = j;
      i += 1;
    }
  }
  var ans = 0;
  var len = (m - 2);
  {
    var i = 0;
    while ((i < s.size()))
    {
      if ((p[i] == len))
      {
        var l = ((i - (2 * len)) - 1);
        var r = ((l + len) + 1);
        if (((l < 0) || (r >= n)))
        {
          i += 1;
          continue;
        }
        if (((((a[l].second == b[0].second) && (a[l].first >= b[0].first)) && (a[r].second == b.back().second)) && (a[r].first >= b.back().first)))
        {
          ans += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  if (0)
  {
    write(fixed, setprecision(0), "TIME = ", ((clock() / cpp_cast(CLOCKS_PER_SEC)) * 1000), "\n");
  }
  return 0;
}
