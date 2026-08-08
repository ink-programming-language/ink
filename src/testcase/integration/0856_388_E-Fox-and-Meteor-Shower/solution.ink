// Translated from solution.cpp.

var n: dynamic;

var ans: dynamic;

var an: dynamic;

var cnt: dynamic;

var ti = cpp_array(1010);

class Xn
{
  var x: dynamic;
  var y: dynamic;
  var vx: dynamic;
  var vy: dynamic;
}

var xn = cpp_array(1010);

var tmp: dynamic;

class Dn
{
  var x: dynamic;
  var y: dynamic;
}

var jd = cpp_array(1010);

func cj(u: dynamic, v: dynamic)
{
  return ((u.x * v.y) - (u.y * v.x));
}

func cmp(u: dynamic, v: dynamic)
{
  if ((fabs(cj(u, v)) < 1e-8))
  {
    if ((fabs((u.x - v.x)) > 1e-8))
    {
      return (u.x < v.x);
    }
    return (u.y < v.y);
  }
  return (cj(u, v) < 0);
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f: dynamic;
  var t: dynamic;
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%Lf%Lf%Lf%Lf%Lf%Lf", (&c), (&a), (&b), (&f), (&d), (&e));
      t = (f - c);
      xn[i].vx = (((d - a)) / t);
      xn[i].vy = (((e - b)) / t);
      xn[i].x = (a - (xn[i].vx * c));
      xn[i].y = (b - (xn[i].vy * c));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      cnt = 0;
      {
        j = 1;
        while ((j <= n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          tmp.x = (xn[i].x - xn[j].x);
          tmp.y = (xn[i].y - xn[j].y);
          tmp.vx = (xn[i].vx - xn[j].vx);
          tmp.vy = (xn[i].vy - xn[j].vy);
          t = if ((fabs(tmp.vx) > 1e-8)) (tmp.x / tmp.vx) else if ((fabs(tmp.vy) > 1e-8)) (tmp.y / tmp.vy) else 0;
          if (((fabs(((tmp.vx * t) - tmp.x)) < 1e-8) && (fabs(((tmp.vy * t) - tmp.y)) < 1e-8)))
          {
            ti[cpp_update(cnt, "++")] = t;
            jd[cnt].x = tmp.vx;
            jd[cnt].y = tmp.vy;
          }
          j += 1;
        }
      }
      if ((!cnt))
      {
        i += 1;
        continue;
      }
      sort((ti + 1), ((ti + cnt) + 1));
      an = 1;
      {
        j = 2;
        while ((j <= cnt))
        {
          if ((fabs((ti[j] - ti[(j - 1)])) < 1e-8))
          {
            an += 1;
          } else
          {
            ans = max(ans, an);
            an = 1;
          }
          j += 1;
        }
      }
      ans = max(an, ans);
      sort((jd + 1), ((jd + cnt) + 1), cmp);
      an = 1;
      {
        j = 2;
        while ((j <= cnt))
        {
          if ((fabs(cj(jd[j], jd[(j - 1)])) < 1e-8))
          {
            if (((fabs((jd[j].x - jd[(j - 1)].x)) > 1e-8) || (fabs((jd[j].y - jd[(j - 1)].y)) > 1e-8)))
            {
              an += 1;
            }
          } else
          {
            ans = max(ans, an);
            an = 1;
          }
          j += 1;
        }
      }
      ans = max(ans, an);
      i += 1;
    }
  }
  write((ans + 1));
}
