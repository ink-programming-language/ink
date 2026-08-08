// Translated from solution.cpp.

var pi = acos(-1);

var INF = 500000;

var N = (100000 + 5);

var l: dynamic;

var m: dynamic;

var n: dynamic;

var a = cpp_array(N);

var b = cpp_array(N);

var c = cpp_array(N);

func init()
{
  read(l, m, n);
  {
    var i = 0;
    while ((i < l))
    {
      read(a[i].first, a[i].second);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      read(b[i].first, b[i].second);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(c[i].first, c[i].second);
      i += 1;
    }
  }
}

func solve_brute()
{
  var cntA: dynamic;
  var cntB: dynamic;
  {
    var i = 0;
    while ((i < l))
    {
      cntA[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      cntB[b[i]] += 1;
      i += 1;
    }
  }
  var ans = 0;
  var a0x = a[0].first;
  var a0y = a[0].second;
  var b0x = b[0].first;
  var b0y = b[0].second;
  var dax = (a[1].first - a[0].first);
  var day = (a[1].second - a[0].second);
  var dbx = (b[1].first - b[0].first);
  var dby = (b[1].second - b[0].second);
  {
    var i = 0;
    while ((i < n))
    {
      var cx = (2 * c[i].first);
      var cy = (2 * c[i].second);
      cx -= (a0x + b0x);
      cy -= (a0y + b0y);
      var det = ((dax * dby) - (dbx * day));
      var ma = ((cx * dby) - (cy * dbx));
      var mb = ((cy * dax) - (cx * day));
      if (((((ma * dax)) % det) != 0))
      {
        i += 1;
        continue;
      }
      if (((((ma * day)) % det) != 0))
      {
        i += 1;
        continue;
      }
      if (((((mb * dbx)) % det) != 0))
      {
        i += 1;
        continue;
      }
      if (((((mb * dby)) % det) != 0))
      {
        i += 1;
        continue;
      }
      var ax = (a0x + ((ma * dax) / det));
      var ay = (a0y + ((ma * day) / det));
      var bx = (b0x + ((mb * dbx) / det));
      var by = (b0y + ((mb * dby) / det));
      ans += (cntA[[ax, ay]] * cntB[[bx, by]]);
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
}

var adic: dynamic;

var bdic: dynamic;

var cdic: dynamic;

func cross(o: dynamic, a: dynamic, b: dynamic)
{
  return ((((a.first - o.first)) * ((b.second - o.second))) - (((a.second - o.second)) * ((b.first - o.first))));
}

func cross(v1: dynamic, v2: dynamic, v3: dynamic, v4: dynamic)
{
  return ((((v2.first - v1.first)) * ((v4.second - v3.second))) - (((v2.second - v1.second)) * ((v4.first - v3.first))));
}

func get_point(v1: dynamic, v2: dynamic, v3: dynamic, v4: dynamic)
{
  var a1 = cross(v3, v1, v4);
  var a2 = cross(v3, v4, v2);
  return wtf([(((v1.first * a2) + (v2.first * a1))), ((a1 + a2))], [(((v1.second * a2) + (v2.second * a1))), ((a1 + a2))]);
}

func dist(v1: dynamic, v2: dynamic)
{
  return sqrt((((ll)((v1.first - v2.first)) * ((v1.first - v2.first))) + ((ll)((v1.second - v2.second)) * ((v1.second - v2.second)))));
}

func rev_bit(bit: dynamic, len: dynamic)
{
  var rev = 0;
  {
    var i = 0;
    while ((((1 << i)) < len))
    {
      rev <<= 1;
      if ((((1 << i)) & bit))
      {
        rev |= 1;
      }
      i += 1;
    }
  }
  return rev;
}

var fin = cpp_array(1200040);

func exec_fft(f: dynamic, len: dynamic, o: dynamic)
{
  {
    var i = 0;
    while ((i < len))
    {
      fin[i] = false;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < len))
    {
      if ((!fin[i]))
      {
        var rev = rev_bit(i, len);
        fin[rev] = cpp_assign(fin[i], "=", true);
        swap(f[rev], f[i]);
      }
      i += 1;
    }
  }
  {
    var s = 2;
    while ((s <= len))
    {
      var mul = cplex(cos((((2 * pi) * o) / s)), sin((((2 * pi) * o) / s)));
      {
        var j = 0;
        while ((j < len))
        {
          var e = cplex(1, 0);
          {
            var k = 0;
            while ((k < ((s >> 1))))
            {
              var t = (f[((j + k) + ((s >> 1)))] * e);
              var u = f[(j + k)];
              f[(j + k)] = (u + t);
              f[((j + k) + ((s >> 1)))] = (u - t);
              e *= mul;
              k += 1;
            }
          }
          j += s;
        }
      }
      s <<= 1;
    }
  }
  if ((o == -1))
  {
    {
      var i = 0;
      while ((i < len))
      {
        f[i] /= cpp_cast(len);
        i += 1;
      }
    }
  }
}

var f = cpp_array(1200040);

var g = cpp_array(1200040);

func solve_pp()
{
  if ((cross(a[0], a[1], c[0], c[1]) == 0))
  {
    var d1 = (abs(cross(a[0], c[0], c[1])) / dist(c[0], c[1]));
    var d2 = (abs(cross(b[0], c[0], c[1])) / dist(c[0], c[1]));
    if ((abs((d1 - d2)) < 1e-10))
    {
      if ((a[0].first != a[1].first))
      {
        {
          var i = 0;
          while ((i < l))
          {
            f[(a[i].first + 100000)] = cplex((f[(a[i].first + 100000)].real() + 1), 0);
            i += 1;
          }
        }
        {
          var i = 0;
          while ((i < m))
          {
            g[(b[i].first + 100000)] = cplex((g[(b[i].first + 100000)].real() + 1), 0);
            i += 1;
          }
        }
        var sz = 1;
        while ((sz <= (400005)))
        {
          sz <<= 1;
        }
        exec_fft(f, sz, 1);
        exec_fft(g, sz, 1);
        {
          var i = 0;
          while ((i < sz))
          {
            f[i] *= g[i];
            i += 1;
          }
        }
        exec_fft(f, sz, -1);
        var ret = 0;
        {
          var i = 0;
          while ((i < n))
          {
            ret += floor((f[((c[i].first * 2) + 200000)].real() + 0.3));
            i += 1;
          }
        }
        return ret;
      } else
      {
        {
          var i = 0;
          while ((i < l))
          {
            f[(a[i].second + 100000)] = cplex((f[(a[i].second + 100000)].real() + 1), 0);
            i += 1;
          }
        }
        {
          var i = 0;
          while ((i < m))
          {
            g[(b[i].second + 100000)] = cplex((g[(b[i].second + 100000)].real() + 1), 0);
            i += 1;
          }
        }
        var sz = 1;
        while ((sz <= (400005)))
        {
          sz <<= 1;
        }
        exec_fft(f, sz, 1);
        exec_fft(g, sz, 1);
        {
          var i = 0;
          while ((i < sz))
          {
            f[i] *= g[i];
            i += 1;
          }
        }
        exec_fft(f, sz, -1);
        var ret = 0;
        {
          var i = 0;
          while ((i < n))
          {
            ret += floor((f[((c[i].second * 2) + 200000)].real() + 0.3));
            i += 1;
          }
        }
        return ret;
      }
    } else
    {
      return 0;
    }
  } else
  {
    var x = get_point(a[0], a[1], c[0], c[1]);
    var y = get_point(b[0], b[1], c[0], c[1]);
    var z = wtf([((x.first.first * y.first.second) + (y.first.first * x.first.second)), (x.first.second * y.first.second)], [((x.second.first * y.second.second) + (x.second.second * y.second.first)), (x.second.second * y.second.second)]);
    {
      var i = 0;
      while ((i < l))
      {
        adic[a[i]] += 1;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < m))
      {
        bdic[b[i]] += 1;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        cdic[c[i]] += 1;
        i += 1;
      }
    }
    var ret = 0;
    {
      var it = adic.begin();
      while ((it != adic.end()))
      {
        if ((((z.first.first - (it->first.first * z.first.second))) % z.first.second))
        {
          it += 1;
          continue;
        }
        if ((((z.second.first - (it->first.second * z.second.second))) % z.second.second))
        {
          it += 1;
          continue;
        }
        ret += (cpp_cast(it->second) * bdic[[(static_cast((z.first.first - (it->first.first * z.first.second))) / z.first.second), (static_cast((z.second.first - (it->first.second * z.second.second))) / z.second.second)]]);
        it += 1;
      }
    }
    return ret;
  }
}

func solve_pin()
{
  write(solve_pp(), cpp_char("\n"));
}

func solve()
{
  sort(a, (a + l));
  sort(b, (b + m));
  sort(c, (c + m));
  if (cpp_binary((a[(l - 1)] == a[0]), "and", (b[(m - 1)] == b[0])))
  {
    var cnt: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        cnt[c[i]] += 1;
        i += 1;
      }
    }
    var dx = ((a[0].first + b[0].first));
    var dy = ((a[0].second + b[0].second));
    if (cpp_binary(((dx & 1)), "or", ((dy & 1))))
    {
      write(0, cpp_char("\n"));
    } else
    {
      dx >>= 1;
      dy >>= 1;
      write(((static_cast(cnt[[dx, dy]]) * l) * m), cpp_char("\n"));
    }
    return;
  } else if (cpp_binary((b[(m - 1)] == b[0]), "and", (c[(n - 1)] == c[0])))
  {
    var cnt: dynamic;
    {
      var i = 0;
      while ((i < l))
      {
        cnt[a[i]] += 1;
        i += 1;
      }
    }
    var dx = ((2 * c[0].first) - b[0].first);
    var dy = ((2 * c[0].second) - b[0].second);
    write(((static_cast(cnt[[dx, dy]]) * m) * n), cpp_char("\n"));
    return;
  } else if (cpp_binary((c[(n - 1)] == c[0]), "and", (a[(l - 1)] == a[0])))
  {
    var cnt: dynamic;
    {
      var i = 0;
      while ((i < m))
      {
        cnt[b[i]] += 1;
        i += 1;
      }
    }
    var dx = ((2 * c[0].first) - a[0].first);
    var dy = ((2 * c[0].second) - a[0].second);
    write(((static_cast(cnt[[dx, dy]]) * n) * l), cpp_char("\n"));
    return;
  }
  if ((a[(l - 1)] == a[0]))
  {
    a[cpp_update(l, "++")] = [INF, INF];
  } else if ((b[(m - 1)] == b[0]))
  {
    b[cpp_update(m, "++")] = [INF, INF];
  } else if ((c[(n - 1)] == c[0]))
  {
    c[cpp_update(n, "++")] = [INF, INF];
  }
  swap(a[1], a[(l - 1)]);
  swap(b[1], b[(m - 1)]);
  swap(c[1], c[(n - 1)]);
  var ax = ((a[0].first - a[1].first));
  var ay = ((a[0].second - a[1].second));
  var bx = ((b[0].first - b[1].first));
  var by = ((b[0].second - b[1].second));
  if ((((ax * by) - (bx * ay)) == 0))
  {
    solve_pin();
  } else
  {
    solve_brute();
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  init();
  solve();
  return 0;
}
