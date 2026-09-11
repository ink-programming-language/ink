// Translated from solution.cpp.

var N: dynamic = 25;

var buf: dynamic = cpp_array(N, N);

var K: dynamic = 8;

var MSK: dynamic = (1 << K);

class vt
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func vt(x: dynamic, y: dynamic) -> dynamic
  {
      x = x;
      y = y;
    }
  func vt() -> dynamic
  {
    }
}

var T: dynamic = cpp_array(K);

var pos: dynamic = cpp_array(K);

var oldi: dynamic = cpp_array(K);

var C: dynamic = cpp_array(K);

var dx: dynamic = 42;

var dy: dynamic = 43;

var D: dynamic = cpp_array(N, N, MSK);

var lpt: dynamic = 0;

var rpt: dynamic = 0;

var Q: dynamic = cpp_array((((10 * MSK) * N) * N));

var vx: dynamic = [1, 0, -1, 0];

var vy: dynamic = [0, 1, 0, -1];

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var pt: dynamic = 0;

func sign(x: dynamic) -> dynamic
{
  return (((x > 0)) - ((x < 0)));
}

func inter(a: dynamic, b: dynamic, c: dynamic, d: dynamic) -> dynamic
{
  return ((((sign((((c - a)) ^ ((b - a)))) * sign((((d - a)) ^ ((b - a))))) == -1) && ((sign((((a - c)) ^ ((d - c)))) * sign((((b - c)) ^ ((d - c))))) == -1)));
}

var aff: dynamic = cpp_array(K, 4, N, N);

func affect(y: dynamic, x: dynamic, v: dynamic, i: dynamic) -> dynamic
{
  if ((aff[y][x][v][i] != -1))
  {
    return aff[y][x][v][i];
  } else
  {
    return cpp_assign(aff[y][x][v][i], "=", inter(vt(y, x), (vt(y, x) + vt(vy[v], vx[v])), pos[i], (pos[i] + vt(dy, dx))));
  }
}

func BFS(sy: dynamic, sx: dynamic) -> dynamic
{
  memset(D, -1, cpp_sizeof((D)));
  D[0][sy][sx] = 0;
  Q[cpp_update(rpt, "++")] = 0;
  Q[cpp_update(rpt, "++")] = sy;
  Q[cpp_update(rpt, "++")] = sx;
  while ((lpt != rpt))
  {
    var msk: dynamic = Q[cpp_update(lpt, "++")];
    var y: dynamic = Q[cpp_update(lpt, "++")];
    var x: dynamic = Q[cpp_update(lpt, "++")];
    {
      var v: dynamic = 0;
      while ((v < 4))
      {
        var ty: dynamic = (y + vy[v]);
        var tx: dynamic = (x + vx[v]);
        if (((((ty < 0) || (tx < 0)) || (ty >= n)) || (tx >= m)))
        {
          v += 1;
          continue;
        }
        if (((buf[ty][tx] != cpp_char(".")) && (buf[ty][tx] != cpp_char("S"))))
        {
          v += 1;
          continue;
        }
        var tmsk: dynamic = msk;
        {
          var i: dynamic = 0;
          while ((i < pt))
          {
            if (affect(y, x, v, i))
            {
              tmsk ^= ((1 << i));
            }
            i += 1;
          }
        }
        if ((D[tmsk][ty][tx] != -1))
        {
          v += 1;
          continue;
        }
        D[tmsk][ty][tx] = (D[msk][y][x] + 1);
        Q[cpp_update(rpt, "++")] = tmsk;
        Q[cpp_update(rpt, "++")] = ty;
        Q[cpp_update(rpt, "++")] = tx;
        v += 1;
      }
    }
  }
}

func main() -> dynamic
{
  memset(aff, -1, cpp_sizeof((aff)));
  scanf("%d %d ", (&n), (&m));
  var sx: dynamic = -1;
  var sy: dynamic = -1;
  var tr: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      gets(buf[i]);
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((buf[i][j] == cpp_char("S")))
          {
            sy = i;
            sx = j;
          } else if ((buf[i][j] == cpp_char("B")))
          {
            T[pt] = 0;
            pos[pt] = vt(i, j);
            pt += 1;
          } else if (((cpp_char("1") <= buf[i][j]) && (buf[i][j] <= cpp_char("8"))))
          {
            T[pt] = 1;
            oldi[pt] = (buf[i][j] - cpp_char("1"));
            pos[pt] = vt(i, j);
            pt += 1;
            tr += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < tr))
    {
      scanf("%d", (&C[i]));
      i += 1;
    }
  }
  assert((sx != -1));
  BFS(sy, sx);
  var ans: dynamic = 0;
  {
    var msk: dynamic = 0;
    while ((msk < ((1 << pt))))
    {
      if ((D[msk][sy][sx] != -1))
      {
        var cst: dynamic = 0;
        var bad: dynamic = false;
        {
          var i: dynamic = 0;
          while ((i < pt))
          {
            if ((((msk >> i)) & 1))
            {
              if ((!T[i]))
              {
                bad = true;
              } else
              {
                cst += C[oldi[i]];
              }
            }
            i += 1;
          }
        }
        if ((!bad))
        {
          ans = max(ans, (cst - D[msk][sy][sx]));
        }
      }
      msk += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
