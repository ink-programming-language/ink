// Translated from solution.cpp.

var N = 25;

var dp = cpp_array(2, 2, 2, N, N, N, N);

var na: dynamic;

var nb: dynamic;

var nc: dynamic;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var A: dynamic;

var B: dynamic;

var C: dynamic;

func pros(x: dynamic)
{
  reverse(x.begin(), x.end());
  x = (cpp_char(" ") + x);
}

class col
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
}

var value = cpp_array(2, 2, 2, N, N, N, N);

class pack
{
  var i: dynamic;
  var posa: dynamic;
  var posb: dynamic;
  var posc: dynamic;
  var carry: dynamic;
  var enda: dynamic;
  var endb: dynamic;
}

var trace = cpp_array(2, 2, 2, N, N, N, N);

var root: dynamic;

func main(argc: dynamic, argv: dynamic)
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  var tmp: dynamic;
  tmp.clear();
  read(t);
  {
    var i = 0;
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
  var res = (N * N);
  {
    var i = 0;
    while ((i <= 20))
    {
      {
        var posa = 0;
        while ((posa <= na))
        {
          {
            var posb = 0;
            while ((posb <= nb))
            {
              {
                var posc = 0;
                while ((posc <= nc))
                {
                  {
                    var carry = 0;
                    while ((carry <= 1))
                    {
                      {
                        var enda = 0;
                        while ((enda <= 1))
                        {
                          {
                            var endb = 0;
                            while ((endb <= 1))
                            {
                              var cur = dp[i][posa][posb][posc][carry][enda][endb];
                              var address = [i, posa, posb, posc, carry, enda, endb];
                              if ((cur == -1))
                              {
                                endb += 1;
                                continue;
                              }
                              {
                                var d1 = 0;
                                while ((d1 <= 9))
                                {
                                  {
                                    var d2 = 0;
                                    while ((d2 <= 9))
                                    {
                                      var s = (((d1 * (if (enda) 0 else 1)) + (d2 * (if (endb) 0 else 1))) + carry);
                                      var nxt = (s % 10);
                                      var ncarry = (s / 10);
                                      var pa = posa;
                                      if ((((!enda) && (posa != na)) && ((a[(posa + 1)] - cpp_char("0")) == d1)))
                                      {
                                        pa += 1;
                                      }
                                      var pb = posb;
                                      if ((((!endb) && (posb != nb)) && ((b[(posb + 1)] - cpp_char("0")) == d2)))
                                      {
                                        pb += 1;
                                      }
                                      var pc = posc;
                                      if (((posc != nc) && ((c[(posc + 1)] - cpp_char("0")) == nxt)))
                                      {
                                        pc += 1;
                                      }
                                      {
                                        var nea = enda;
                                        while ((nea <= 1))
                                        {
                                          {
                                            var neb = endb;
                                            while ((neb <= 1))
                                            {
                                              var state = dp[(i + 1)][pa][pb][pc][ncarry][nea][neb];
                                              if (((state == -1) || (state > (((cur + 3) - enda) - endb))))
                                              {
                                                state = (((cur + 3) - enda) - endb);
                                                trace[(i + 1)][pa][pb][pc][ncarry][nea][neb] = address;
                                                value[(i + 1)][pa][pb][pc][ncarry][nea][neb] = [(if (enda) -1 else d1), (if (endb) -1 else d2), nxt];
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
  var leaf = [0, 0, 0, 0, 0, 0, 0];
  while ((((((((root.i + root.posa) + root.posb) + root.posc) + root.carry) + root.enda) + root.endb) != 0))
  {
    var D = value[root.i][root.posa][root.posb][root.posc][root.carry][root.enda][root.endb];
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
