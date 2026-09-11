// Translated from solution.cpp.

var N: dynamic = 25;

var dp: dynamic = cpp_array(2, 2, 2, N, N, N, N);

var na: dynamic = cpp_uninitialized();

var nb: dynamic = cpp_uninitialized();

var nc: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

func pros(x: dynamic) -> dynamic
{
  reverse(x.begin(), x.end());
  x = (cpp_char(" ") + x);
}

class col
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
}

var value: dynamic = cpp_array(2, 2, 2, N, N, N, N);

class pack
{
  var i: dynamic = cpp_uninitialized();
  var posa: dynamic = cpp_uninitialized();
  var posb: dynamic = cpp_uninitialized();
  var posc: dynamic = cpp_uninitialized();
  var carry: dynamic = cpp_uninitialized();
  var enda: dynamic = cpp_uninitialized();
  var endb: dynamic = cpp_uninitialized();
}

var trace: dynamic = cpp_array(2, 2, 2, N, N, N, N);

var root: dynamic = cpp_uninitialized();

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  tmp.clear();
  read(t);
  {
    var i: dynamic = 0;
    while ((i < t.size()))
    {
      if (((t[i] >= cpp_char("0")) && (t[i] <= cpp_char("9"))))
      {
        tmp += t[i];
      } else
      {
        if ((t[i] == cpp_char("+")))
        {
          a = tmp;
        }
        if ((t[i] == cpp_char("=")))
        {
          b = tmp;
        }
        tmp.clear();
      }
      i += 1;
    }
  }
  c = tmp;
  na = a.size();
  nb = b.size();
  nc = c.size();
  pros(a);
  pros(b);
  pros(c);
  memset(dp, -1, cpp_sizeof(dp));
  dp[0][0][0][0][0][0][0] = 0;
  var res: dynamic = (N * N);
  {
    var i: dynamic = 0;
    while ((i <= 20))
    {
      {
        var posa: dynamic = 0;
        while ((posa <= na))
        {
          {
            var posb: dynamic = 0;
            while ((posb <= nb))
            {
              {
                var posc: dynamic = 0;
                while ((posc <= nc))
                {
                  {
                    var carry: dynamic = 0;
                    while ((carry <= 1))
                    {
                      {
                        var enda: dynamic = 0;
                        while ((enda <= 1))
                        {
                          {
                            var endb: dynamic = 0;
                            while ((endb <= 1))
                            {
                              var cur: dynamic = dp[i][posa][posb][posc][carry][enda][endb];
                              var address: dynamic = [i, posa, posb, posc, carry, enda, endb];
                              if ((cur == -1))
                              {
                                endb += 1;
                                continue;
                              }
                              {
                                var d1: dynamic = 0;
                                while ((d1 <= 9))
                                {
                                  {
                                    var d2: dynamic = 0;
                                    while ((d2 <= 9))
                                    {
                                      var s: dynamic = (((d1 * ( (enda) ? 0 : 1)) + (d2 * ( (endb) ? 0 : 1))) + carry);
                                      var nxt: dynamic = (s % 10);
                                      var ncarry: dynamic = (s / 10);
                                      var pa: dynamic = posa;
                                      if ((((!enda) && (posa != na)) && ((a[(posa + 1)] - cpp_char("0")) == d1)))
                                      {
                                        pa += 1;
                                      }
                                      var pb: dynamic = posb;
                                      if ((((!endb) && (posb != nb)) && ((b[(posb + 1)] - cpp_char("0")) == d2)))
                                      {
                                        pb += 1;
                                      }
                                      var pc: dynamic = posc;
                                      if (((posc != nc) && ((c[(posc + 1)] - cpp_char("0")) == nxt)))
                                      {
                                        pc += 1;
                                      }
                                      {
                                        var nea: dynamic = enda;
                                        while ((nea <= 1))
                                        {
                                          {
                                            var neb: dynamic = endb;
                                            while ((neb <= 1))
                                            {
                                              var state: dynamic = dp[(i + 1)][pa][pb][pc][ncarry][nea][neb];
                                              if (((state == -1) || (state > (((cur + 3) - enda) - endb))))
                                              {
                                                state = (((cur + 3) - enda) - endb);
                                                trace[(i + 1)][pa][pb][pc][ncarry][nea][neb] = address;
                                                value[(i + 1)][pa][pb][pc][ncarry][nea][neb] = [( (enda) ? -1 : d1), ( (endb) ? -1 : d2), nxt];
                                                neb += 1;
                                                continue;
                                              }
                                              state = min(state, (((cur + 3) - enda) - endb));
                                              neb += 1;
                                            }
                                          }
                                          nea += 1;
                                        }
                                      }
                                      d2 += 1;
                                    }
                                  }
                                  d1 += 1;
                                }
                              }
                              if ((((((posa == na) && (posb == nb)) && (posc == nc)) && (res > cur)) && (carry == 0)))
                              {
                                res = cur;
                                root = address;
                              }
                              endb += 1;
                            }
                          }
                          enda += 1;
                        }
                      }
                      carry += 1;
                    }
                  }
                  posc += 1;
                }
              }
              posb += 1;
            }
          }
          posa += 1;
        }
      }
      i += 1;
    }
  }
  var leaf: dynamic = [0, 0, 0, 0, 0, 0, 0];
  while ((((((((root.i + root.posa) + root.posb) + root.posc) + root.carry) + root.enda) + root.endb) != 0))
  {
    var D: dynamic = value[root.i][root.posa][root.posb][root.posc][root.carry][root.enda][root.endb];
    if ((D.a != -1))
    {
      A += char((cpp_char("0") + D.a));
    }
    if ((D.b != -1))
    {
      B += char((cpp_char("0") + D.b));
    }
    C += char((cpp_char("0") + D.c));
    root = trace[root.i][root.posa][root.posb][root.posc][root.carry][root.enda][root.endb];
  }
  write(A, "+", B, "=", C, "\n");
  return 0;
}
