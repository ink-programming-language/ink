// Translated from solution.cpp.

var wmem: dynamic;

var memarr = cpp_array(96000000);

func min_L(a: dynamic, b: dynamic)
{
  return if ((a <= b)) a else b;
}

func walloc1d(arr: dynamic, x: dynamic, mem: dynamic = (&wmem))
{
  var skip = [0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1];
  ((*mem)) = cpp_cast((((cpp_cast(((*mem)))) + skip[((cpp_cast(((*mem)))) & 15)])));
  ((*arr)) = cpp_cast(((*mem)));
  ((*mem)) = ((((*arr)) + x));
}

func walloc1d(arr: dynamic, x1: dynamic, x2: dynamic, mem: dynamic = (&wmem))
{
  walloc1d(arr, (x2 - x1), mem);
  ((*arr)) -= x1;
}

func sortA_L(N: dynamic, a: dynamic, mem: dynamic = wmem)
{
  sort(a, (a + N));
}

func sortA_L(N: dynamic, a: dynamic, b: dynamic, mem: dynamic = wmem)
{
  var i: dynamic;
  var arr: dynamic;
  walloc1d((&arr), N, (&mem));
  {
    i = (0);
    while ((i < (N)))
    {
      arr[i].first = a[i];
      arr[i].second = b[i];
      i += 1;
    }
  }
  sort(arr, (arr + N));
  {
    i = (0);
    while ((i < (N)))
    {
      a[i] = arr[i].first;
      b[i] = arr[i].second;
      i += 1;
    }
  }
}

func my_getchar()
{
  var buf = cpp_array(1048576);
  var s = 1048576;
  var e = 1048576;
  if (((s == e) && (e == 1048576)))
  {
    e = fread(buf, 1, 1048576, stdin);
    s = 0;
  }
  if ((s == e))
  {
    return EOF;
  }
  return buf[cpp_update(s, "++")];
}

func rd(x: dynamic)
{
  var k: dynamic;
  var m = 0;
  x = 0;
  {
    while (true)
    {
      k = my_getchar();
      if ((k == cpp_char("-")))
      {
        m = 1;
        break;
      }
      if (((cpp_char("0") <= k) && (k <= cpp_char("9"))))
      {
        x = (k - cpp_char("0"));
        break;
      }
    }
  }
  {
    while (true)
    {
      k = my_getchar();
      if (((k < cpp_char("0")) || (k > cpp_char("9"))))
      {
        break;
      }
      x = (((x * 10) + k) - cpp_char("0"));
    }
  }
  if (m)
  {
    x = (-x);
  }
}

func rd(x: dynamic)
{
  var k: dynamic;
  var m = 0;
  x = 0;
  {
    while (true)
    {
      k = my_getchar();
      if ((k == cpp_char("-")))
      {
        m = 1;
        break;
      }
      if (((cpp_char("0") <= k) && (k <= cpp_char("9"))))
      {
        x = (k - cpp_char("0"));
        break;
      }
    }
  }
  {
    while (true)
    {
      k = my_getchar();
      if (((k < cpp_char("0")) || (k > cpp_char("9"))))
      {
        break;
      }
      x = (((x * 10) + k) - cpp_char("0"));
    }
  }
  if (m)
  {
    x = (-x);
  }
}

func rd_int(argument_0: dynamic)
{
  var x: dynamic;
  rd(x);
  return x;
}

class MY_WRITER
{
  var buf: dynamic = cpp_array(1048576);
  var s: dynamic;
  var e: dynamic;
  func MY_WRITER()
  {
      s = 0;
      e = 1048576;
    }
  func ~MY_WRITER()
  {
      if (s)
      {
        fwrite(buf, 1, s, stdout);
      }
    }
}

var MY_WRITER_VAR: dynamic;

func my_putchar(a: dynamic)
{
  if ((MY_WRITER_VAR.s == MY_WRITER_VAR.e))
  {
    fwrite(MY_WRITER_VAR.buf, 1, MY_WRITER_VAR.s, stdout);
    MY_WRITER_VAR.s = 0;
  }
  MY_WRITER_VAR.buf[cpp_update(MY_WRITER_VAR.s, "++")] = a;
}

func wt_L(a: dynamic)
{
  my_putchar(a);
}

func wt_L(x: dynamic)
{
  var s = 0;
  var m = 0;
  var f = cpp_array(10);
  if ((x < 0))
  {
    m = 1;
    x = (-x);
  }
  while (x)
  {
    f[cpp_update(s, "++")] = (x % 10);
    x /= 10;
  }
  if ((!s))
  {
    f[cpp_update(s, "++")] = 0;
  }
  if (m)
  {
    my_putchar(cpp_char("-"));
  }
  while (cpp_update(s, "--"))
  {
    my_putchar((f[s] + cpp_char("0")));
  }
}

func arrInsert(k: dynamic, sz: dynamic, a: dynamic, aval: dynamic)
{
  var i: dynamic;
  sz += 1;
  {
    i = (sz - 1);
    while ((i > k))
    {
      a[i] = a[(i - 1)];
      i -= 1;
    }
  }
  a[k] = aval;
}

func arrInsert(k: dynamic, sz: dynamic, a: dynamic, aval: dynamic, b: dynamic, bval: dynamic)
{
  var i: dynamic;
  sz += 1;
  {
    i = (sz - 1);
    while ((i > k))
    {
      a[i] = a[(i - 1)];
      i -= 1;
    }
  }
  {
    i = (sz - 1);
    while ((i > k))
    {
      b[i] = b[(i - 1)];
      i -= 1;
    }
  }
  a[k] = aval;
  b[k] = bval;
}

func arrInsert(k: dynamic, sz: dynamic, a: dynamic, aval: dynamic, b: dynamic, bval: dynamic, c: dynamic, cval: dynamic)
{
  var i: dynamic;
  sz += 1;
  {
    i = (sz - 1);
    while ((i > k))
    {
      a[i] = a[(i - 1)];
      i -= 1;
    }
  }
  {
    i = (sz - 1);
    while ((i > k))
    {
      b[i] = b[(i - 1)];
      i -= 1;
    }
  }
  {
    i = (sz - 1);
    while ((i > k))
    {
      c[i] = c[(i - 1)];
      i -= 1;
    }
  }
  a[k] = aval;
  b[k] = bval;
  c[k] = cval;
}

func arrInsert(k: dynamic, sz: dynamic, a: dynamic, aval: dynamic, b: dynamic, bval: dynamic, c: dynamic, cval: dynamic, d: dynamic, dval: dynamic)
{
  var i: dynamic;
  sz += 1;
  {
    i = (sz - 1);
    while ((i > k))
    {
      a[i] = a[(i - 1)];
      i -= 1;
    }
  }
  {
    i = (sz - 1);
    while ((i > k))
    {
      b[i] = b[(i - 1)];
      i -= 1;
    }
  }
  {
    i = (sz - 1);
    while ((i > k))
    {
      c[i] = c[(i - 1)];
      i -= 1;
    }
  }
  {
    i = (sz - 1);
    while ((i > k))
    {
      d[i] = d[(i - 1)];
      i -= 1;
    }
  }
  a[k] = aval;
  b[k] = bval;
  c[k] = cval;
  d[k] = dval;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
  return a;
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
  return a;
}

var N: dynamic;

var C: dynamic;

var Q: dynamic;

var A = cpp_array(30000);

var X: dynamic;

var Y: dynamic;

var cnt = cpp_array((30000 + 2), 5);

var sz: dynamic;

var lis = cpp_array(5);

var ind = cpp_array(5);

var usz = cpp_array((30000 + 2));

var ulis = cpp_array(5, (30000 + 2));

var dsz = cpp_array((30000 + 2));

var dlis = cpp_array(5, (30000 + 2));

var skipL = cpp_array((30000 + 2), 5);

var skipR = cpp_array((30000 + 2), 5);

var skipV = cpp_array((30000 + 2), 5);

var skip = 150;

var skip2L = cpp_array((30000 + 2), 5);

var skip2R = cpp_array((30000 + 2), 5);

var skip2V = cpp_array((30000 + 2), 5);

var skip2 = 30;

func main()
{
  var t_ynMSdg: dynamic;
  wmem = memarr;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var c: dynamic;
  {
    i = (0);
    while ((i < (5)))
    {
      cnt[i][0] = 1;
      i += 1;
    }
  }
  {
    i = (0);
    while ((i < (5)))
    {
      {
        k = (0);
        while ((k < (30000)))
        {
          {
            j = (0);
            while ((j < ((min_L(k, i) + 1))))
            {
              cnt[i][(k + 1)] += cnt[(i - j)][(k - j)];
              if ((cnt[i][(k + 1)] > 2000000000000000000))
              {
                cnt[i][(k + 1)] = 2000000000000000000;
              }
              j += 1;
            }
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  var KrdatlYV = rd_int();
  {
    t_ynMSdg = (0);
    while ((t_ynMSdg < (KrdatlYV)))
    {
      var dtiCQK_a: dynamic;
      rd(N);
      rd(C);
      rd(Q);
      {
        var a2conNHc: dynamic;
        {
          a2conNHc = (0);
          while ((a2conNHc < (N)))
          {
            rd(A[a2conNHc]);
            a2conNHc += 1;
          }
        }
      }
      {
        k = (0);
        while ((k < (N)))
        {
          sz = 0;
          {
            i = (0);
            while ((i < ((C + 1))))
            {
              if (((k + i) < N))
              {
                arrInsert(sz, sz, ind, i, lis, A[(k + i)]);
              }
              i += 1;
            }
          }
          sortA_L(sz, lis, ind);
          j = cpp_assign(usz[k], "=", cpp_assign(dsz[k], "=", 0));
          {
            i = (0);
            while ((i < (sz)))
            {
              if ((ind[i] == 0))
              {
                j += 1;
                i += 1;
                continue;
              }
              if ((j == 0))
              {
                ulis[k][cpp_update(usz[k], "++")] = ind[i];
              }
              if ((j == 1))
              {
                dlis[k][cpp_update(dsz[k], "++")] = ind[i];
              }
              i += 1;
            }
          }
          k += 1;
        }
      }
      {
        k = (0);
        while ((k < (N)))
        {
          {
            c = (0);
            while ((c < ((C + 1))))
            {
              var sm = 0;
              skipL[c][k] = -4611686016279904256;
              skipR[c][k] = 4611686016279904256;
              if ((((k + skip) + 5) >= N))
              {
                swap(skipL[c][k], skipR[c][k]);
                c += 1;
                continue;
              }
              {
                j = (k);
                while ((j < ((k + skip))))
                {
                  {
                    i = (0);
                    while ((i < (usz[j])))
                    {
                      if ((c >= ulis[j][i]))
                      {
                        sm += cnt[(c - ulis[j][i])][(((N - j) - ulis[j][i]) - 1)];
                      }
                      i += 1;
                    }
                  }
                  if ((sm > 4611686016279904256))
                  {
                    sm = 2000000000000000000;
                  }
                  chmax(skipL[c][k], sm);
                  chmin(skipR[c][k], (sm + cnt[c][((N - j) - 1)]));
                  j += 1;
                }
              }
              skipV[c][k] = sm;
              c += 1;
            }
          }
          k += 1;
        }
      }
      {
        k = (0);
        while ((k < (N)))
        {
          {
            c = (0);
            while ((c < ((C + 1))))
            {
              var sm = 0;
              skip2L[c][k] = -4611686016279904256;
              skip2R[c][k] = 4611686016279904256;
              if ((((k + skip2) + 5) >= N))
              {
                swap(skip2L[c][k], skip2R[c][k]);
                c += 1;
                continue;
              }
              {
                j = (k);
                while ((j < ((k + skip2))))
                {
                  {
                    i = (0);
                    while ((i < (usz[j])))
                    {
                      if ((c >= ulis[j][i]))
                      {
                        sm += cnt[(c - ulis[j][i])][(((N - j) - ulis[j][i]) - 1)];
                      }
                      i += 1;
                    }
                  }
                  if ((sm > 4611686016279904256))
                  {
                    sm = 2000000000000000000;
                  }
                  chmax(skip2L[c][k], sm);
                  chmin(skip2R[c][k], (sm + cnt[c][((N - j) - 1)]));
                  j += 1;
                }
              }
              skip2V[c][k] = sm;
              c += 1;
            }
          }
          k += 1;
        }
      }
      {
        dtiCQK_a = (0);
        while ((dtiCQK_a < (Q)))
        {
          rd(X);
          X += (-1);
          rd(Y);
          Y += (-1);
          if ((Y >= cnt[C][N]))
          {
            wt_L(-1);
            wt_L(cpp_char("\n"));
            dtiCQK_a += 1;
            continue;
          }
          c = C;
          {
            k = (0);
            while ((k < (N)))
            {
              if (((skipL[c][k] <= Y) && (Y < skipR[c][k])))
              {
                if ((X < (k + skip)))
                {
                  wt_L(A[X]);
                  wt_L(cpp_char("\n"));
                  break;
                }
                Y -= skipV[c][k];
                k += (skip - 1);
                k += 1;
                continue;
              }
              if (((skip2L[c][k] <= Y) && (Y < skip2R[c][k])))
              {
                if ((X < (k + skip2)))
                {
                  wt_L(A[X]);
                  wt_L(cpp_char("\n"));
                  break;
                }
                Y -= skip2V[c][k];
                k += (skip2 - 1);
                k += 1;
                continue;
              }
              {
                i = (0);
                while ((i < (usz[k])))
                {
                  if ((c >= ulis[k][i]))
                  {
                    if ((Y < cnt[(c - ulis[k][i])][(((N - k) - ulis[k][i]) - 1)]))
                    {
                      if ((X <= (k + ulis[k][i])))
                      {
                        wt_L(A[((k + ulis[k][i]) - ((X - k)))]);
                        wt_L(cpp_char("\n"));
                        cpp_goto("goto qE8LMwYZ;");
                      }
                      c -= ulis[k][i];
                      k += ulis[k][i];
                      cpp_goto("goto lQU550vz;");
                    } else
                    {
                      Y -= cnt[(c - ulis[k][i])][(((N - k) - ulis[k][i]) - 1)];
                    }
                  }
                  i += 1;
                }
              }
              if ((Y < cnt[c][((N - k) - 1)]))
              {
                if ((X == k))
                {
                  wt_L(A[k]);
                  wt_L(cpp_char("\n"));
                  break;
                }
                k += 1;
                continue;
              } else
              {
                Y -= cnt[c][((N - k) - 1)];
              }
              {
                i = (0);
                while ((i < (dsz[k])))
                {
                  if ((c >= dlis[k][i]))
                  {
                    if ((Y < cnt[(c - dlis[k][i])][(((N - k) - dlis[k][i]) - 1)]))
                    {
                      if ((X <= (k + dlis[k][i])))
                      {
                        wt_L(A[((k + dlis[k][i]) - ((X - k)))]);
                        wt_L(cpp_char("\n"));
                        cpp_goto("goto qE8LMwYZ;");
                      }
                      c -= dlis[k][i];
                      k += dlis[k][i];
                      cpp_goto("goto lQU550vz;");
                    } else
                    {
                      Y -= cnt[(c - dlis[k][i])][(((N - k) - dlis[k][i]) - 1)];
                    }
                  }
                  i += 1;
                }
              }
              k += 1;
            }
          }
          dtiCQK_a += 1;
        }
      }
      t_ynMSdg += 1;
    }
  }
  return 0;
}
