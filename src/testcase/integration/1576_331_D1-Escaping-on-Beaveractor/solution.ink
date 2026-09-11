// Translated from solution.cpp.

func setmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
  }
}

func setmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    a = b;
  }
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [1, 0, -1, 0];

var dc: dynamic = [cpp_char("U"), cpp_char("R"), cpp_char("D"), cpp_char("L")];

var MAXN: dynamic = 100010;

var MAXQ: dynamic = 100010;

var MAXLog: dynamic = 20;

var ax0: dynamic = cpp_array(MAXN);

var ay0: dynamic = cpp_array(MAXN);

var ax1: dynamic = cpp_array(MAXN);

var ay1: dynamic = cpp_array(MAXN);

var avx: dynamic = cpp_array(MAXN);

var avy: dynamic = cpp_array(MAXN);

var nxt: dynamic = cpp_array(MAXN);

var dis: dynamic = cpp_array(MAXN);

var qx: dynamic = cpp_array(MAXN);

var qy: dynamic = cpp_array(MAXN);

var qd: dynamic = cpp_array(MAXN);

var qt: dynamic = cpp_array(MAXN);

var qnxt: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_uninitialized();

var qs: dynamic = cpp_uninitialized();

func sgn(x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return 0;
  } else
  {
    return  ((x > 0)) ? 1 : -1;
  }
}

var lst: dynamic = cpp_array((MAXN + MAXQ));

var color: dynamic = cpp_array((MAXN * 4));

func paint(x: dynamic, s: dynamic, t: dynamic, le: dynamic, ri: dynamic, c: dynamic) -> dynamic
{
  if (((le <= s) && (t <= ri)))
  {
    color[x] = c;
    return;
  }
  var mid: dynamic = (((s + t)) / 2);
  if ((color[x] >= 0))
  {
    color[(x * 2)] = cpp_assign(color[((x * 2) + 1)], "=", color[x]);
    color[x] = -1;
  }
  if ((le <= mid))
  {
    paint((x * 2), s, mid, le, ri, c);
  }
  if ((mid < ri))
  {
    paint(((x * 2) + 1), (mid + 1), t, le, ri, c);
  }
}

func getcolor(x: dynamic, s: dynamic, t: dynamic, p: dynamic) -> dynamic
{
  if ((color[x] >= 0))
  {
    return color[x];
  }
  var mid: dynamic = (((s + t)) / 2);
  if ((p <= mid))
  {
    return getcolor((x * 2), s, mid, p);
  } else
  {
    return getcolor(((x * 2) + 1), (mid + 1), t, p);
  }
}

func init() -> dynamic
{
  scanf("%d%d", (&n), (&sz));
  {
    var i: dynamic = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      scanf("%d%d%d%d", (ax0 + i), (ay0 + i), (ax1 + i), (ay1 + i));
      avx[i] = sgn((ax1[i] - ax0[i]));
      avy[i] = sgn((ay1[i] - ay0[i]));
      i += 1;
    }
  }
  scanf("%d", (&qs));
  {
    var i: dynamic = cpp_cast((1));
    while ((i <= cpp_cast((qs))))
    {
      var c: dynamic = cpp_array(9);
      scanf("%d%d%s", (qx + i), (qy + i), c);
      read(qt[i]);
      qd[i] = 0;
      while ((dc[qd[i]] != c[0]))
      {
        qd[i] += 1;
      }
      i += 1;
    }
  }
  {
    var dir: dynamic = 0;
    while ((dir < cpp_cast((4))))
    {
      {
        var i: dynamic = cpp_cast((1));
        while ((i <= cpp_cast((n))))
        {
          var v0: dynamic = ((ax0[i] * dx[dir]) + (ay0[i] * dy[dir]));
          var v1: dynamic = ((ax1[i] * dx[dir]) + (ay1[i] * dy[dir]));
          lst[i].first = min((-v0), (-v1));
          lst[i].second = (-i);
          i += 1;
        }
      }
      {
        var i: dynamic = cpp_cast((1));
        while ((i <= cpp_cast((qs))))
        {
          lst[(n + i)].first = (-(((qx[i] * dx[dir]) + (qy[i] * dy[dir]))));
          lst[(n + i)].second = i;
          i += 1;
        }
      }
      sort((lst + 1), (((lst + n) + qs) + 1));
      memset(color, 0, cpp_sizeof((color)));
      {
        var k: dynamic = cpp_cast((1));
        while ((k <= cpp_cast(((n + qs)))))
        {
          var i: dynamic = (-lst[k].second);
          if ((i > 0))
          {
            if (((avx[i] == dx[dir]) && (avy[i] == dy[dir])))
            {
              var s: dynamic = ax1[i];
              if ((dx[dir] != 0))
              {
                s = ay1[i];
              }
              var c: dynamic = getcolor(1, 1, (sz + 1), (s + 1));
              nxt[i] = c;
            }
            var l: dynamic = ax0[i];
            var r: dynamic = ax1[i];
            if ((dx[dir] != 0))
            {
              l = ay0[i];
              r = ay1[i];
            }
            if ((l > r))
            {
              swap(l, r);
            }
            paint(1, 1, (sz + 1), (l + 1), (r + 1), i);
          } else
          {
            i = (-i);
            if ((dir == qd[i]))
            {
              var s: dynamic = qx[i];
              if ((dx[dir] != 0))
              {
                s = qy[i];
              }
              qnxt[i] = getcolor(1, 1, (sz + 1), (s + 1));
            }
          }
          k += 1;
        }
      }
      dir += 1;
    }
  }
  {
    var i: dynamic = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if ((nxt[i] == 0))
      {
        var sid: dynamic = 0;
        if (((avx[i] + avy[i]) > 0))
        {
          sid = sz;
        }
        dis[i] = abs((sid - ( ((avx[i] != 0)) ? ax1[i] : ay1[i])));
      } else
      {
        var j: dynamic = nxt[i];
        dis[i] = (abs((ax1[i] - ax1[j])) + abs((ay1[i] - ay1[j])));
      }
      i += 1;
    }
  }
}

var clen: dynamic = cpp_array(MAXN);

var vis: dynamic = cpp_array(MAXN);

var tmp: dynamic = cpp_array(MAXN);

var plnk: dynamic = cpp_array(MAXLog, MAXN);

var plen: dynamic = cpp_array(MAXLog, MAXN);

func getwalk(t: dynamic, x1: dynamic, y1: dynamic, dx: dynamic, dy: dynamic, x2: dynamic, y2: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  var fl: dynamic = false;
  if ((dx == 0))
  {
    fl = true;
    swap(x1, y1);
    swap(x2, y2);
    swap(dx, dy);
  }
  var px: dynamic = abs((x1 - x2));
  if ((t <= px))
  {
    x = (x1 + (dx * t));
    y = y1;
  } else
  {
    x = x2;
    y = y1;
    t -= px;
    dy = sgn((y2 - y1));
    y += (dy * t);
  }
  if (fl)
  {
    swap(x, y);
  }
}

func onside(x: dynamic, y: dynamic, x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic) -> dynamic
{
  if ((x1 > x2))
  {
    swap(x1, x2);
  }
  if ((y1 > y2))
  {
    swap(y1, y2);
  }
  return ((((x >= x1) && (x <= x2)) && (y >= y1)) && (y <= y2));
}

func solve() -> dynamic
{
  memset(clen, 0xff, cpp_sizeof((clen)));
  memset(tmp, 0xff, cpp_sizeof((tmp)));
  memset(vis, false, cpp_sizeof((vis)));
  {
    var i: dynamic = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if ((!vis[i]))
      {
        tmp[i] = 0;
        {
          var y: dynamic = i;
          var x: dynamic = nxt[i];
          while (true)
          {
            if ((x == 0))
            {
              break;
            }
            if (vis[x])
            {
              break;
            }
            if ((tmp[x] >= 0))
            {
              var len: dynamic = ((tmp[y] + dis[y]) - tmp[x]);
              clen[x] = len;
              {
                var k: dynamic = nxt[x];
                while ((k != x))
                {
                  clen[k] = len;
                  k = nxt[k];
                }
              }
              break;
            }
            tmp[x] = (tmp[y] + dis[y]);
            y = x;
            x = nxt[x];
          }
        }
        {
          var x: dynamic = i;
          while (((x > 0) && (!vis[x])))
          {
            vis[x] = true;
            x = nxt[x];
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if ((nxt[i] > 0))
      {
        plnk[i][0] = nxt[i];
        plen[i][0] = dis[i];
      } else
      {
        plnk[i][0] = i;
        plen[i][0] = 0;
      }
      i += 1;
    }
  }
  {
    var j: dynamic = cpp_cast((1));
    while ((j <= cpp_cast(((MAXLog - 1)))))
    {
      {
        var i: dynamic = cpp_cast((1));
        while ((i <= cpp_cast((n))))
        {
          plnk[i][j] = plnk[plnk[i][(j - 1)]][(j - 1)];
          plen[i][j] = (plen[i][(j - 1)] + plen[plnk[i][(j - 1)]][(j - 1)]);
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var cur: dynamic = cpp_cast((1));
    while ((cur <= cpp_cast((qs))))
    {
      var d: dynamic = qd[cur];
      if ((qnxt[cur] == 0))
      {
        var x: dynamic = (qx[cur] + (dx[d] * qt[cur]));
        var y: dynamic = (qy[cur] + (dy[d] * qt[cur]));
        if ((x < 0))
        {
          x = 0;
        }
        if ((x > sz))
        {
          x = sz;
        }
        if ((y < 0))
        {
          y = 0;
        }
        if ((y > sz))
        {
          y = sz;
        }
        printf("%d %d\n", cpp_cast(x), cpp_cast(y));
        cur += 1;
        continue;
      }
      var i: dynamic = qnxt[cur];
      var l: dynamic = (abs((qx[cur] - ax1[i])) + abs((qy[cur] - ay1[i])));
      var t: dynamic = qt[cur];
      if ((t <= l))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        var u: dynamic = dx[d];
        var v: dynamic = dy[d];
        if (onside(qx[cur], qy[cur], ax0[i], ay0[i], ax1[i], ay1[i]))
        {
          u = avx[i];
          v = avy[i];
        }
        getwalk(cpp_cast(t), qx[cur], qy[cur], u, v, ax1[i], ay1[i], x, y);
        printf("%d %d\n", x, y);
      } else
      {
        t -= l;
        {
          var j: dynamic = (MAXLog - 1);
          while ((j >= 0))
          {
            if ((plen[i][j] <= t))
            {
              t -= plen[i][j];
              i = plnk[i][j];
            }
            j -= 1;
          }
        }
        if ((clen[i] > 0))
        {
          t %= clen[i];
        }
        {
          var j: dynamic = (MAXLog - 1);
          while ((j >= 0))
          {
            if ((plen[i][j] <= t))
            {
              t -= plen[i][j];
              i = plnk[i][j];
            }
            j -= 1;
          }
        }
        var j: dynamic = nxt[i];
        if ((j == 0))
        {
          var x: dynamic = (ax1[i] + (avx[i] * t));
          var y: dynamic = (ay1[i] + (avy[i] * t));
          if ((x < 0))
          {
            x = 0;
          }
          if ((x > sz))
          {
            x = sz;
          }
          if ((y < 0))
          {
            y = 0;
          }
          if ((y > sz))
          {
            y = sz;
          }
          printf("%d %d\n", cpp_cast(x), cpp_cast(y));
        } else
        {
          var x: dynamic = cpp_uninitialized();
          var y: dynamic = cpp_uninitialized();
          getwalk(cpp_cast(t), ax1[i], ay1[i], avx[i], avy[i], ax1[j], ay1[j], x, y);
          printf("%d %d\n", x, y);
        }
      }
      cur += 1;
    }
  }
}

func main() -> dynamic
{
  init();
  solve();
  return 0;
}
