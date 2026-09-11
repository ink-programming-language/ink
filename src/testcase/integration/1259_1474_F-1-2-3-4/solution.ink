// Translated from solution.cpp.

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << p.first) << ", ") << p.second) << cpp_char(")"));
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << cpp_char("{"));
  var sep: dynamic = cpp_uninitialized();
  for (var x: dynamic in v)
  {
    ((os << sep) << x);
    sep = ", ";
  }
  return (os << cpp_char("}"));
}

func dbg_out() -> dynamic
{
  write("\n");
}

func dbg_out(H: dynamic, T: dynamic...) -> dynamic
{
  write(cpp_char(" "), H);
  dbg_out(cpp_expand(T));
}

func dbg() -> dynamic
{
  return cpp_expression("#include <algorithm> #include <array> #include <cassert>");
}

func dbg() -> dynamic
{
  cpp_macro("");
}

class m_int
{
  var val: dynamic = cpp_uninitialized();
  func m_int(v: dynamic = 0) -> dynamic
  {
      if ((v < 0))
      {
        v = ((v % MOD) + MOD);
      }
      if ((v >= MOD))
      {
        v %= MOD;
      }
      val = int_cpp(v);
    }
  func m_int(v: dynamic) -> dynamic
  {
      if ((v >= MOD))
      {
        v %= MOD;
      }
      val = int_cpp(v);
    }
  func m_int(v: dynamic) -> dynamic
  {
      cpp_base_construct(int64_t(v));
    }
  func m_int(v: dynamic) -> dynamic
  {
      cpp_base_construct(uint64_t(v));
    }
  func cpp_function_1() -> dynamic
  {
      return val;
    }
  func cpp_function_2() -> dynamic
  {
      return val;
    }
  func cpp_function_3() -> dynamic
  {
      return val;
    }
  func cpp_function_4() -> dynamic
  {
      return val;
    }
  func cpp_function_5() -> dynamic
  {
      return val;
    }
  func cpp_function_6() -> dynamic
  {
      return val;
    }
  func operator_add_assign(other: dynamic) -> dynamic
  {
      val -= (MOD - other.val);
      if ((val < 0))
      {
        val += MOD;
      }
      return (*self);
    }
  func operator_subtract_assign(other: dynamic) -> dynamic
  {
      val -= other.val;
      if ((val < 0))
      {
        val += MOD;
      }
      return (*self);
    }
  func fast_mod(x: dynamic, m: dynamic = MOD) -> dynamic
  {
      cpp_statement("!defined(_WIN32) || defined(_WIN64)");
      return unsigned((x % m));
      var x_high: dynamic = unsigned((x >> 32));
      var x_low: dynamic = unsigned(x);
      var quot: dynamic = cpp_uninitialized();
      var rem: dynamic = cpp_uninitialized();
      cpp_expression("asm(\"divl %4\\n\" : \"=a\" (quot), \"=d\" (rem) : \"d\" (x_high), \"a\" (x_low), \"r\" (m))");
      return rem;
    }
  func operator(other: dynamic) -> dynamic
  {
      val = fast_mod((uint64_t(val) * other.val));
      return (*self);
    }
  func operator(other: dynamic) -> dynamic
  {
      return cpp_assign((*self), "*=", other.inv());
    }
  func operator() -> dynamic
  {
      val =  ((val == (MOD - 1))) ? 0 : (val + 1);
      return (*self);
    }
  func operator() -> dynamic
  {
      val =  ((val == 0)) ? (MOD - 1) : (val - 1);
      return (*self);
    }
  func operator(argument_0: dynamic) -> dynamic
  {
      var before: dynamic = (*self);
      (*self) += 1;
      return before;
    }
  func operator(argument_0: dynamic) -> dynamic
  {
      var before: dynamic = (*self);
      (*self) -= 1;
      return before;
    }
  func operator_subtract() -> dynamic
  {
      return  ((val == 0)) ? 0 : (MOD - val);
    }
  var SAVE_INV: dynamic = cpp_uninitialized();
  var save_inv: dynamic = cpp_array(SAVE_INV);
  func prepare_inv() -> dynamic
  {
      {
        var p: dynamic = 2;
        while (((p * p) <= MOD))
        {
          assert(((MOD % p) != 0));
          p += ((p % 2) + 1);
        }
      }
      save_inv[0] = 0;
      save_inv[1] = 1;
      {
        var i: dynamic = 2;
        while ((i < SAVE_INV))
        {
          save_inv[i] = (save_inv[(MOD % i)] * ((MOD - (MOD / i))));
          i += 1;
        }
      }
    }
  func inv() -> dynamic
  {
      if ((save_inv[1] == 0))
      {
        prepare_inv();
      }
      if ((val < SAVE_INV))
      {
        return save_inv[val];
      }
      var product: dynamic = 1;
      var v: dynamic = val;
      while ((v >= SAVE_INV))
      {
        product *= (MOD - (MOD / v));
        v = (MOD % v);
      }
      return (product * save_inv[v]);
    }
  func pow(p: dynamic) -> dynamic
  {
      if ((p < 0))
      {
        return inv().pow((-p));
      }
      var a: dynamic = (*self);
      var result: dynamic = 1;
      while ((p > 0))
      {
        if ((p & 1))
        {
          result *= a;
        }
        p >>= 1;
        if ((p > 0))
        {
          a *= a;
        }
      }
      return result;
    }
}

var save_inv: dynamic = cpp_array(m_int.SAVE_INV);

var MOD: dynamic = 998244353;

var factorial: dynamic = [1, 1];

var inv_factorial: dynamic = [1, 1];

func prepare_factorials(maximum: dynamic) -> dynamic
{
  var prepared_maximum: dynamic = 1;
  if ((maximum <= prepared_maximum))
  {
    return;
  }
  maximum += (maximum / 16);
  factorial.resize((maximum + 1));
  inv_factorial.resize((maximum + 1));
  {
    var i: dynamic = (prepared_maximum + 1);
    while ((i <= maximum))
    {
      factorial[i] = (i * factorial[(i - 1)]);
      inv_factorial[i] = (inv_factorial[(i - 1)] / i);
      i += 1;
    }
  }
  prepared_maximum = int_cpp(maximum);
}

func factorial(n: dynamic) -> dynamic
{
  if ((n < 0))
  {
    return 0;
  }
  prepare_factorials(n);
  return factorial[n];
}

func inv_factorial(n: dynamic) -> dynamic
{
  if ((n < 0))
  {
    return 0;
  }
  prepare_factorials(n);
  return inv_factorial[n];
}

func choose(n: dynamic, r: dynamic) -> dynamic
{
  if (((r < 0) || (r > n)))
  {
    return 0;
  }
  prepare_factorials(n);
  return ((factorial[n] * inv_factorial[r]) * inv_factorial[(n - r)]);
}

func permute(n: dynamic, r: dynamic) -> dynamic
{
  if (((r < 0) || (r > n)))
  {
    return 0;
  }
  prepare_factorials(n);
  return (factorial[n] * inv_factorial[(n - r)]);
}

func inv_choose(n: dynamic, r: dynamic) -> dynamic
{
  assert(((0 <= r) && (r <= n)));
  prepare_factorials(n);
  return ((inv_factorial[n] * factorial[r]) * factorial[(n - r)]);
}

func inv_permute(n: dynamic, r: dynamic) -> dynamic
{
  assert(((0 <= r) && (r <= n)));
  prepare_factorials(n);
  return (inv_factorial[n] * factorial[(n - r)]);
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var cpp_name: dynamic = cpp_uninitialized();
  read(N, cpp_name);
  for (var d: dynamic in D)
  {
    read(d);
  }
  if (((*max_element(D.begin(), D.end())) <= 0))
  {
    write(1, cpp_char(" "), ((-accumulate(D.begin(), D.end(), mod_int(0))) + 1), cpp_char("\n"));
    return 0;
  }
  D.erase(remove(D.begin(), D.end(), 0), D.end());
  var sorted: dynamic = [0];
  for (var d: dynamic in D)
  {
    sorted.push_back((sorted.back() + d));
  }
  sort(sorted.begin(), sorted.end());
  sorted.erase(unique(sorted.begin(), sorted.end()), sorted.end());
  var ON: dynamic = N;
  N = int_cpp(sorted.size());
  var sequence: dynamic = [0];
  var get_index: dynamic = __cpp_lambda_7;
  for (var d: dynamic in D)
  {
    var it: dynamic = lower_bound(sorted.begin(), sorted.end(), sequence.back());
    var target: dynamic = (sequence.back() + d);
    if ((d > 0))
    {
      while (((*it) != target))
      {
        sequence.push_back((*cpp_update(it, "++")));
      }
    } else
    {
      while (((*it) != target))
      {
        sequence.push_back((*cpp_update(it, "--")));
      }
    }
  }
  var S: dynamic = int_cpp(sequence.size());
  var longest: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < S))
    {
      {
        var j: dynamic = i;
        while ((j < S))
        {
          longest = max(longest, (sequence[j] - sequence[i]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var choose_loop: dynamic = __cpp_lambda_8;
  var total: dynamic = 0;
  {
    var initial: dynamic = 0;
    while ((initial < N))
    {
      var goal: dynamic = get_index((sorted[initial] + longest));
      if (((goal >= N) || (sorted[goal] != (sorted[initial] + longest))))
      {
        initial += 1;
        continue;
      }
      var ready_for: dynamic = cpp_construct(N, 0);
      var dp: dynamic = cpp_construct((N + 1), vector((ON + 1), vector((ON + 1), 0)));
      ready_for[initial] = 1;
      var recompute_ready_for: dynamic = __cpp_lambda_9;
      {
        var i: dynamic = 0;
        while ((i < (S - 1)))
        {
          if ((sequence[i] == sorted[goal]))
          {
            total += ready_for[goal];
          }
          var smaller: dynamic = min(sequence[i], sequence[(i + 1)]);
          var bigger: dynamic = max(sequence[i], sequence[(i + 1)]);
          var s_index: dynamic = get_index(smaller);
          var b_index: dynamic = get_index(bigger);
          assert((b_index == (s_index + 1)));
          if ((sequence[i] < sequence[(i + 1)]))
          {
            {
              var p: dynamic = (ON - 1);
              while ((p >= 0))
              {
                {
                  var n: dynamic = ((ON - 1) - p);
                  while ((n >= 0))
                  {
                    dp[s_index][(p + 1)][n] += dp[s_index][p][n];
                    n -= 1;
                  }
                }
                p -= 1;
              }
            }
            dp[s_index][1][0] += ready_for[s_index];
            recompute_ready_for(b_index);
          } else
          {
            {
              var p: dynamic = (ON - 1);
              while ((p >= 0))
              {
                {
                  var n: dynamic = ((ON - 1) - p);
                  while ((n >= 0))
                  {
                    dp[s_index][p][(n + 1)] += dp[s_index][p][n];
                    n -= 1;
                  }
                }
                p -= 1;
              }
            }
            if (((i == 0) || (sequence[(i - 1)] < sequence[i])))
            {
              dp[b_index][0][1] += ready_for[b_index];
            }
            if ((((i + 2) < S) && (sequence[(i + 1)] > sequence[(i + 2)])))
            {
              dp[s_index][0][1] += ready_for[s_index];
            }
          }
          i += 1;
        }
      }
      if ((sequence.back() == sorted[goal]))
      {
        total += ready_for[goal];
      }
      initial += 1;
    }
  }
  write((longest + 1), cpp_char(" "), total, cpp_char("\n"));
}

func __cpp_lambda_7(value: dynamic) -> dynamic
{
  return int_cpp((lower_bound(sorted.begin(), sorted.end(), value) - sorted.begin()));
}

func __cpp_lambda_8(n: dynamic, r: dynamic) -> dynamic
{
  if (((r < 0) || (r > n)))
  {
    return 0;
  }
  var product: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < r))
    {
      product *= (n - i);
      i += 1;
    }
  }
  product *= inv_factorial(int_cpp(r));
  return product;
}

func __cpp_lambda_9(index: dynamic) -> dynamic
{
  if (((index <= initial) || (index > goal)))
  {
    return;
  }
  ready_for[index] = 0;
  var diff: dynamic = (sorted[index] - sorted[(index - 1)]);
  {
    var p: dynamic = 0;
    while ((p <= ON))
    {
      {
        var n: dynamic = 0;
        while (((p + n) <= ON))
        {
          ready_for[index] += (dp[(index - 1)][p][n] * ( ((p == 0)) ? ( ((n == diff)) ? 1 : 0) : choose_loop(((diff - n) - 1), (p - 1))));
          n += 1;
        }
      }
      p += 1;
    }
  }
}
