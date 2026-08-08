// Translated from solution.cpp.

func lowbit(x: dynamic)
{
  return (x & (-x));
}

func h_bit(x: dynamic)
{
  return int_cpp(((cpp_sizeof(dynamic) * 8) - builtin_clzll(x)));
}

func pow2(x: dynamic)
{
  return if ((x == lowbit(x))) x else (1 << h_bit(x));
}

func get_bit(a: dynamic, i: dynamic)
{
  return ((a >> i) & 1);
}

func get_mid(l: dynamic, r: dynamic)
{
  assert((l <= r));
  return (l + (((r - l) >> 1)));
}

func to_string(s: dynamic)
{
  return ((cpp_char("\"") + s) + cpp_char("\""));
}

func to_string(s: dynamic)
{
  return to_string(cpp_cast(s));
}

func to_string(b: dynamic)
{
  return (if (b) "true" else "false");
}

func to_string(p: dynamic)
{
  return (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
}

func to_string(bs: dynamic)
{
  return bs.to_string();
}

func to_string(v: dynamic)
{
  var first = true;
  var res = "{";
  for (var x in v)
  {
    if ((!first))
    {
      res += ", ";
    }
    first = false;
    res += to_string(x);
  }
  res += "}";
  return res;
}

func debug_out()
{
  write("\n");
}

func debug_out(H: dynamic, T: dynamic...)
{
  write(" ", to_string(H));
  debug_out(cpp_expand(T));
}

class fast_ios
{
  func fast_ios()
  {
      cin.tie(null);
      ios.sync_with_stdio(false);
      write(fixed, setprecision(10));
    }
}

var fast_ios: dynamic;

func operator_shift_right(stream: dynamic, vec: dynamic)
{
  for (var x in vec)
  {
    (stream >> x);
  }
  return stream;
}

func operator_shift_right(in_cpp: dynamic, p: dynamic)
{
  ((in_cpp >> p.first) >> p.second);
  return in_cpp;
}

func scan()
{
}

func scan(a: dynamic, rest: dynamic...)
{
  read(a);
  scan(cpp_expand(rest));
}

func operator_shift_left(stream: dynamic, vec: dynamic)
{
  var first = true;
  for (var t in vec)
  {
    if (first)
    {
      first = false;
    } else
    {
      write(cpp_char(" "));
    }
    write(t);
  }
  return stream;
}

func operator_shift_left(out: dynamic, p: dynamic)
{
  (((out << p.first) << cpp_char(" ")) << p.second);
  return out;
}

func print(t: dynamic)
{
  for (var row in t)
  {
    write(row, cpp_char("\n"));
  }
}

func print(t: dynamic)
{
  write(t, cpp_char(" "));
}

func print(t: dynamic, rest: dynamic...)
{
  print(t);
  print(cpp_expand(rest));
}

func println(t: dynamic)
{
  write(t, cpp_char("\n"));
}

func println(t: dynamic, rest: dynamic...)
{
  print(t);
  println(cpp_expand(rest));
}

func chkmin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return true;
  }
  return false;
}

func chkmax(a: dynamic, b: dynamic)
{
  if ((b > a))
  {
    a = b;
    return true;
  }
  return false;
}

func ceil(x: dynamic, y: dynamic)
{
  assert((y > 0));
  if ((x > 0))
  {
    x += (y - 1);
  }
  return (x / y);
}

func floor(x: dynamic, y: dynamic)
{
  assert((y > 0));
  if ((x < 0))
  {
    x -= (y - 1);
  }
  return (x / y);
}

var dp = cpp_array((1 << 15), 15, 16);

var pre = cpp_array((1 << 15), 15, 16);

class FMakeItAscending
{
  func solve(argument_0: dynamic, argument_1: dynamic)
  {
      var T: dynamic;
      scan(T);
      {
        var iter_212 = 0;
        var num_212 = T;
        while ((iter_212 < num_212))
        {
          var n: dynamic;
          scan(n);
          scan(a);
          var tot = (1 << n);
          var all = (tot - 1);
          {
            var i = 1;
            while ((i <= n))
            {
              {
                var j = 0;
                while ((j < n))
                {
                  {
                    var s = 0;
                    while ((s < tot))
                    {
                      dp[i][j][s] = 0;
                      s += 1;
                    }
                  }
                  j += 1;
                }
              }
              i += 1;
            }
          }
          {
            var i = 0;
            while ((i < tot))
            {
              {
                var j = 0;
                while ((j < n))
                {
                  if (get_bit(i, j))
                  {
                    sum[i] += a[j];
                  }
                  j += 1;
                }
              }
              i += 1;
            }
          }
          {
            var s = 1;
            while ((s < tot))
            {
              var i = builtin_ctz(s);
              dp[1][i][s] = sum[s];
              pre[1][i][s] = [0, -1];
              s += 1;
            }
          }
          var ans: dynamic;
          {
            var i = 1;
            while ((i < n))
            {
              {
                var j = 0;
                while ((j < (n - 1)))
                {
                  {
                    var s = 1;
                    while ((s < (1 << n)))
                    {
                      if (dp[i][j][s])
                      {
                        {
                          var u = cpp_binary(all, "xor", s);
                          var t = u;
                          while (((t > 0) && ((h_bit(t) - 1) > j)))
                          {
                            if ((sum[t] <= dp[i][j][s]))
                            {
                              t = ((t - 1) & u);
                              continue;
                            }
                            {
                              var k = (j + 1);
                              while ((k < n))
                              {
                                if (get_bit(t, k))
                                {
                                  var tar = dp[(i + 1)][k][(s | t)];
                                  if (cpp_binary((tar == 0), "or", (tar > sum[t])))
                                  {
                                    tar = sum[t];
                                    pre[(i + 1)][k][(s | t)] = [s, j];
                                  }
                                  break;
                                }
                                k += 1;
                              }
                            }
                            t = ((t - 1) & u);
                          }
                        }
                      }
                      s += 1;
                    }
                  }
                  j += 1;
                }
              }
              var flag = false;
              {
                var j = 0;
                while ((j < n))
                {
                  if (dp[(i + 1)][j][all])
                  {
                    flag = true;
                    break;
                  }
                  j += 1;
                }
              }
              if ((!flag))
              {
                {
                  var j = 0;
                  while ((j < n))
                  {
                    if (dp[i][j][all])
                    {
                      var s = all;
                      {
                        var k = i;
                        while ((k >= 1))
                        {
                          var (ps, pj) = pre[k][j][s];
                          var t = (s ^ ps);
                          {
                            var l = 0;
                            while ((l < n))
                            {
                              if ((get_bit(t, l) && (l != j)))
                              {
                                ans.emplace_back(l, j);
                              }
                              l += 1;
                            }
                          }
                          s = ps;
                          j = pj;
                          k -= 1;
                        }
                      }
                      break;
                    }
                    j += 1;
                  }
                }
                break;
              }
              i += 1;
            }
          }
          println(cpp_cast((ans).size()));
          var get_index = __cpp_lambda_1;
          for (var p in ans)
          {
            println(get_index(p.first), get_index(p.second));
            removed[p.first] = true;
          }
          iter_212 += 1;
        }
      }
    }
}

func main()
{
  var solver: dynamic;
  solver.solve(0, 0);
  return 0;
}

func __cpp_lambda_1(i: dynamic)
{
  var res = 0;
  {
    var j = 0;
    while ((j < i))
    {
      if ((!removed[j]))
      {
        res += 1;
      }
      j += 1;
    }
  }
  return (res + 1);
}
