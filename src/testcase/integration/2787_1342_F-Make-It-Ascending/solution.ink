// Translated from solution.cpp.

func lowbit(x: dynamic) -> dynamic
{
  return (x & (-x));
}

func h_bit(x: dynamic) -> dynamic
{
  return int_cpp(((cpp_sizeof(dynamic) * 8) - builtin_clzll(x)));
}

func pow2(x: dynamic) -> dynamic
{
  return  ((x == lowbit(x))) ? x : (1 << h_bit(x));
}

func get_bit(a: dynamic, i: dynamic) -> dynamic
{
  return ((a >> i) & 1);
}

func get_mid(l: dynamic, r: dynamic) -> dynamic
{
  assert((l <= r));
  return (l + (((r - l) >> 1)));
}

func to_string(s: dynamic) -> dynamic
{
  return ((cpp_char("\"") + s) + cpp_char("\""));
}

func to_string(s: dynamic) -> dynamic
{
  return to_string(cpp_cast(s));
}

func to_string(b: dynamic) -> dynamic
{
  return ( (b) ? "true" : "false");
}

func to_string(p: dynamic) -> dynamic
{
  return (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
}

func to_string(bs: dynamic) -> dynamic
{
  return bs.to_string();
}

func to_string(v: dynamic) -> dynamic
{
  var first: dynamic = true;
  var res: dynamic = "{";
  for (var x: dynamic in v)
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

func debug_out() -> dynamic
{
  write("\n");
}

func debug_out(H: dynamic, T: dynamic...) -> dynamic
{
  write(" ", to_string(H));
  debug_out(cpp_expand(T));
}

class fast_ios
{
  func fast_ios() -> dynamic
  {
      cin.tie(null);
      ios.sync_with_stdio(false);
      write(fixed, setprecision(10));
    }
}

var fast_ios: dynamic = cpp_uninitialized();

func operator_shift_right(stream: dynamic, vec: dynamic) -> dynamic
{
  for (var x: dynamic in vec)
  {
    (stream >> x);
  }
  return stream;
}

func operator_shift_right(in_cpp: dynamic, p: dynamic) -> dynamic
{
  ((in_cpp >> p.first) >> p.second);
  return in_cpp;
}

func scan() -> dynamic
{
}

func scan(a: dynamic, rest: dynamic...) -> dynamic
{
  read(a);
  scan(cpp_expand(rest));
}

func operator_shift_left(stream: dynamic, vec: dynamic) -> dynamic
{
  var first: dynamic = true;
  for (var t: dynamic in vec)
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

func operator_shift_left(out: dynamic, p: dynamic) -> dynamic
{
  (((out << p.first) << cpp_char(" ")) << p.second);
  return out;
}

func print(t: dynamic) -> dynamic
{
  for (var row: dynamic in t)
  {
    write(row, cpp_char("\n"));
  }
}

func print(t: dynamic) -> dynamic
{
  write(t, cpp_char(" "));
}

func print(t: dynamic, rest: dynamic...) -> dynamic
{
  print(t);
  print(cpp_expand(rest));
}

func println(t: dynamic) -> dynamic
{
  write(t, cpp_char("\n"));
}

func println(t: dynamic, rest: dynamic...) -> dynamic
{
  print(t);
  println(cpp_expand(rest));
}

func chkmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return true;
  }
  return false;
}

func chkmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    a = b;
    return true;
  }
  return false;
}

func ceil(x: dynamic, y: dynamic) -> dynamic
{
  assert((y > 0));
  if ((x > 0))
  {
    x += (y - 1);
  }
  return (x / y);
}

func floor(x: dynamic, y: dynamic) -> dynamic
{
  assert((y > 0));
  if ((x < 0))
  {
    x -= (y - 1);
  }
  return (x / y);
}

var dp: dynamic = cpp_array((1 << 15), 15, 16);

var pre: dynamic = cpp_array((1 << 15), 15, 16);

class FMakeItAscending
{
  func solve(argument_0: dynamic, argument_1: dynamic) -> dynamic
  {
      var T: dynamic = cpp_uninitialized();
      scan(T);
      {
        var iter_212: dynamic = 0;
        var num_212: dynamic = T;
        while ((iter_212 < num_212))
        {
          var n: dynamic = cpp_uninitialized();
          scan(n);
          scan(a);
          var tot: dynamic = (1 << n);
          var all: dynamic = (tot - 1);
          {
            var i: dynamic = 1;
            while ((i <= n))
            {
              {
                var j: dynamic = 0;
                while ((j < n))
                {
                  {
                    var s: dynamic = 0;
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
            var i: dynamic = 0;
            while ((i < tot))
            {
              {
                var j: dynamic = 0;
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
            var s: dynamic = 1;
            while ((s < tot))
            {
              var i: dynamic = builtin_ctz(s);
              dp[1][i][s] = sum[s];
              pre[1][i][s] = [0, -1];
              s += 1;
            }
          }
          var ans: dynamic = cpp_uninitialized();
          {
            var i: dynamic = 1;
            while ((i < n))
            {
              {
                var j: dynamic = 0;
                while ((j < (n - 1)))
                {
                  {
                    var s: dynamic = 1;
                    while ((s < (1 << n)))
                    {
                      if (dp[i][j][s])
                      {
                        {
                          var u: dynamic = cpp_binary(all, "xor", s);
                          var t: dynamic = u;
                          while (((t > 0) && ((h_bit(t) - 1) > j)))
                          {
                            if ((sum[t] <= dp[i][j][s]))
                            {
                              t = ((t - 1) & u);
                              continue;
                            }
                            {
                              var k: dynamic = (j + 1);
                              while ((k < n))
                              {
                                if (get_bit(t, k))
                                {
                                  var tar: dynamic = dp[(i + 1)][k][(s | t)];
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
              var flag: dynamic = false;
              {
                var j: dynamic = 0;
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
                  var j: dynamic = 0;
                  while ((j < n))
                  {
                    if (dp[i][j][all])
                    {
                      var s: dynamic = all;
                      {
                        var k: dynamic = i;
                        while ((k >= 1))
                        {
                          var (ps, pj): dynamic = pre[k][j][s];
                          var t: dynamic = (s ^ ps);
                          {
                            var l: dynamic = 0;
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
          var get_index: dynamic = __cpp_lambda_1;
          for (var p: dynamic in ans)
          {
            println(get_index(p.first), get_index(p.second));
            removed[p.first] = true;
          }
          iter_212 += 1;
        }
      }
    }
}

func main() -> dynamic
{
  var solver: dynamic = cpp_uninitialized();
  solver.solve(0, 0);
  return 0;
}

func __cpp_lambda_1(i: dynamic) -> dynamic
{
  var res: dynamic = 0;
  {
    var j: dynamic = 0;
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
