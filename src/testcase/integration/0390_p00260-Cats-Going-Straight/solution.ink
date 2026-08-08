// Translated from solution.cpp.

func EQ(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<bits/stdc++");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(n);i++)");
}

var fs = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var pb = cpp_expression("#include<");

var sz = cpp_expression("#inclu");

func all(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

var EPS = 1e-8;

var PI = acos(-1);

func operator_less(a: dynamic, b: dynamic)
{
  return if (EQ(real(a), real(b))) (imag(a) < imag(b)) else (real(a) < real(b));
}

func operator_equal(a: dynamic, b: dynamic)
{
  return EQ(a, b);
}

func dot(x: dynamic, y: dynamic)
{
  return real((conj(x) * y));
}

func cross(x: dynamic, y: dynamic)
{
  return imag((conj(x) * y));
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  b -= a;
  c -= a;
  if ((cross(b, c) > EPS))
  {
    return 1;
  }
  if ((cross(b, c) < (-EPS)))
  {
    return -1;
  }
  if ((dot(b, c) < (-EPS)))
  {
    return 2;
  }
  if ((abs(b) < abs(c)))
  {
    return -2;
  }
  return 0;
}

func para(a: dynamic, b: dynamic)
{
  return (abs(cross((a.fs - a.sc), (b.fs - b.sc))) < EPS);
}

func line_cp(a: dynamic, b: dynamic)
{
  return (a.fs + ((((a.sc - a.fs)) * cross((b.sc - b.fs), (b.fs - a.fs))) / cross((b.sc - b.fs), (a.sc - a.fs))));
}

func is_cp(a: dynamic, b: dynamic)
{
  if (((ccw(a.fs, a.sc, b.fs) * ccw(a.fs, a.sc, b.sc)) <= 0))
  {
    if (((ccw(b.fs, b.sc, a.fs) * ccw(b.fs, b.sc, a.sc)) <= 0))
    {
      return true;
    }
  }
  return false;
}

func in_poly(p: dynamic, x: dynamic)
{
  if (p.empty())
  {
    return false;
  }
  var s = p.size();
  var xMax = x.real();
  var h = L(x, P((xMax + 1.0), x.imag()));
  var c = 0;
  return if (((c & 1))) true else false;
}

func main()
{
  var n: dynamic;
  var p = cpp_array(20);
  var l = cpp_array(20);
  while (cpp_comma((cin >> n), n))
  {
    var poly: dynamic;
    rep(i, n)[i] = L(p[i], p[(((i + 1)) % n)]);
    var segs: dynamic;
    rep(i, n);
    rep(j, segs.size());
    {
      var lay1 = L(p[i], segs[j].fs);
      var lay2 = L(p[i], segs[j].sc);
      var f = false;
      if (f)
      {
        continue;
      }
      var mp = ((1.0 / 3) * (((p[i] + lay1.sc) + lay2.sc)));
      if ((f || in_poly(poly, mp)))
      {
        visible[i].push_back(j);
      }
    }
    var ans = n;
    rep(i, (1 << n));
    {
      var ok = cpp_construct(segs.size(), false);
      var cnt = 0;
      var f = true;
      rep(j, ok.size()) &= ok[j];
      if (f)
      {
        ans = min(ans, cnt);
      }
    }
    write(ans, "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((xMax < p[i].real()))
    {
      xMax = p[i].real();
    }
    if (EQ(x, p[i]))
    {
      return false;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var l = L(p[i], p[(((i + 1)) % s)]);
    if (((!para(h, l)) && is_cp(h, l)))
    {
      var cp = line_cp(h, l);
      if ((cp.real() < (x.real() + EPS)))
      {
        continue;
      }
      if ((!EQ(cp, if (((l.fs.imag() < l.sc.imag()))) l.sc else l.fs)))
      {
        c += 1;
      }
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      p[i] = P(x, y);
      poly.push_back(p[i]);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((j == k))
        {
          continue;
        }
        var l2 = L(p[j], p[k]);
        if ((!para(l[i], l2)))
        {
          var cp = line_cp(l[i], l2);
          if ((!ccw(l[i].fs, l[i].sc, cp)))
          {
            cut_point.push_back(cp);
          }
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var cut_point: dynamic;
      rep(j, n);
      cut_point.push_back(l[i].fs);
      cut_point.push_back(l[i].sc);
      sort(all(cut_point));
      {
        var i = 1;
        while ((i < cpp_cast(cut_point.size())))
        {
          if (EQ(cut_point[(i - 1)], cut_point[i]))
          {
            i += 1;
            continue;
          }
          segs.push_back(L(cut_point[(i - 1)], cut_point[i]));
          i += 1;
        }
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((!para(lay1, l[k])))
        {
          var cp1 = line_cp(lay1, l[k]);
          if (((((((!EQ(lay1.fs, cp1)) && (!EQ(lay1.sc, cp1))) && (!EQ(l[k].fs, cp1))) && (!EQ(l[k].sc, cp1))) && (!ccw(lay1.fs, lay1.sc, cp1))) && (!ccw(l[k].fs, l[k].sc, cp1))))
          {
            f = true;
          }
        }
        if ((!para(lay2, l[k])))
        {
          var cp2 = line_cp(lay2, l[k]);
          if (((((((!EQ(lay2.fs, cp2)) && (!EQ(lay2.sc, cp2))) && (!EQ(l[k].fs, cp2))) && (!EQ(l[k].sc, cp2))) && (!ccw(lay2.fs, lay2.sc, cp2))) && (!ccw(l[k].fs, l[k].sc, cp2))))
          {
            f = true;
          }
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((!ccw(l[k].fs, l[k].sc, mp)))
        {
          f = true;
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((((i >> j)) & 1))
        {
          rep(k, visible[j].size())[visible[j][k]] = true;
          cnt += 1;
        }
      }
