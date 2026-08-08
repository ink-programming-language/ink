// Translated from solution.cpp.

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << cpp_char("(")) << p.first) << ", ") << p.second) << cpp_char(")"));
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << cpp_char("{"));
  var sep: dynamic;
  for (var x in v)
  {
    ((os << sep) << x);
    sep = ", ";
  }
  return (os << cpp_char("}"));
}

func dbg_out()
{
  write("\n");
}

func dbg_out(H: dynamic, T: dynamic...)
{
  write(cpp_char(" "), H);
  dbg_out(cpp_expand(T));
}

func dbg()
{
  return cpp_expression("#include <algorithm> #include <array> #include <cassert>");
}

func dbg()
{
  cpp_macro("");
}

class m_int
{
  var val: dynamic;
  func m_int(v: dynamic = 0)
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
  func m_int(v: dynamic)
  {
      if ((v >= MOD))
      {
        v %= MOD;
      }
      val = int_cpp(v);
    }
  func m_int(v: dynamic)
  {
      cpp_base_construct(int64_t(v));
    }
  func m_int(v: dynamic)
  {
      cpp_base_construct(uint64_t(v));
    }
  func cpp_function_1()
  {
      return val;
    }
  func cpp_function_2()
  {
      return val;
    }
  func cpp_function_3()
  {
      return val;
    }
  func cpp_function_4()
  {
      return val;
    }
  func cpp_function_5()
  {
      return val;
    }
  func cpp_function_6()
  {
      return val;
    }
  func operator_add_assign(other: dynamic)
  {
      val -= (MOD - other.val);
      if ((val < 0))
      {
        val += MOD;
      }
      return (*this);
    }
  func operator_subtract_assign(other: dynamic)
  {
      val -= other.val;
      if ((val < 0))
      {
        val += MOD;
      }
      return (*this);
    }
  func fast_mod(x: dynamic, m: dynamic = MOD)
  {
      cpp_statement("!defined(_WIN32) || defined(_WIN64)");
      return unsigned((x % m));
      var x_high = unsigned((x >> 32));
      var x_low = unsigned(x);
      var quot: dynamic;
      var rem: dynamic;
      cpp_expression("asm(\"divl %4\\n\" : \"=a\" (quot), \"=d\" (rem) : \"d\" (x_high), \"a\" (x_low), \"r\" (m))");
      return rem;
    }
  func operator(other: dynamic)
  {
      val = fast_mod((uint64_t(val) * other.val));
      return (*this);
    }
  func operator(other: dynamic)
  {
      return cpp_assign((*this), "*=", other.inv());
    }
  func operator()
  {
      val = if ((val == (MOD - 1))) 0 else (val + 1);
      return (*this);
    }
  func operator()
  {
      val = if ((val == 0)) (MOD - 1) else (val - 1);
      return (*this);
    }
  func operator(argument_0: dynamic)
  {
      var before = (*this);
      (*this) += 1;
      return before;
    }
  func operator(argument_0: dynamic)
  {
      var before = (*this);
      (*this) -= 1;
      return before;
    }
  func operator_subtract()
  {
      return if ((val == 0)) 0 else (MOD - val);
    }
  var SAVE_INV: dynamic;
  var save_inv: dynamic = cpp_array(SAVE_INV);
  func prepare_inv()
  {
      {
        var p = 2;
        while (((p * p) <= MOD))
        {
          assert(((MOD % p) != 0));
          p += ((p % 2) + 1);
        }
      }
      save_inv[0] = 0;
      save_inv[1] = 1;
      {
        var i = 2;
        while ((i < SAVE_INV))
        {
          save_inv[i] = (save_inv[(MOD % i)] * ((MOD - (MOD / i))));
          i += 1;
        }
      }
    }
  func inv()
  {
      if ((save_inv[1] == 0))
      {
        prepare_inv();
      }
      if ((val < SAVE_INV))
      {
        return save_inv[val];
      }
      var product = 1;
      var v = val;
      while ((v >= SAVE_INV))
      {
        product *= (MOD - (MOD / v));
        v = (MOD % v);
      }
      return (product * save_inv[v]);
    }
  func pow(p: dynamic)
  {
      if ((p < 0))
      {
        return inv().pow((-p));
      }
      var a = (*this);
      var result = 1;
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

var save_inv = cpp_array(m_int.SAVE_INV);

var MOD = 998244353;

var factorial = [1, 1];

var inv_factorial = [1, 1];

func prepare_factorials(maximum: dynamic)
{
  var prepared_maximum = 1;
  if ((maximum <= prepared_maximum))
  {
    return;
  }
  maximum += (maximum / 16);
  factorial.resize((maximum + 1));
  inv_factorial.resize((maximum + 1));
  {
    var i = (prepared_maximum + 1);
    while ((i <= maximum))
    {
      factorial[i] = (i * factorial[(i - 1)]);
      inv_factorial[i] = (inv_factorial[(i - 1)] / i);
      i += 1;
    }
  }
  prepared_maximum = int_cpp(maximum);
}

func factorial(n: dynamic)
{
  if ((n < 0))
  {
    return 0;
  }
  prepare_factorials(n);
  return factorial[n];
}

func inv_factorial(n: dynamic)
{
  if ((n < 0))
  {
    return 0;
  }
  prepare_factorials(n);
  return inv_factorial[n];
}

func choose(n: dynamic, r: dynamic)
{
  if (((r < 0) || (r > n)))
  {
    return 0;
  }
  prepare_factorials(n);
  return ((factorial[n] * inv_factorial[r]) * inv_factorial[(n - r)]);
}

func permute(n: dynamic, r: dynamic)
{
  if (((r < 0) || (r > n)))
  {
    return 0;
  }
  prepare_factorials(n);
  return (factorial[n] * inv_factorial[(n - r)]);
}

func inv_choose(n: dynamic, r: dynamic)
{
  assert(((0 <= r) && (r <= n)));
  prepare_factorials(n);
  return ((inv_factorial[n] * factorial[r]) * factorial[(n - r)]);
}

func inv_permute(n: dynamic, r: dynamic)
{
  assert(((0 <= r) && (r <= n)));
  prepare_factorials(n);
  return (inv_factorial[n] * factorial[(n - r)]);
}

func main()
{
  var N: dynamic;
  var cpp_name: dynamic;
  read(N, cpp_name);
  for (var d in D)
  {
    read(d);
  }
  if (((*max_element(D.begin(), D.end())) <= 0))
  {
    write(1, cpp_char(" "), ((-accumulate(D.begin(), D.end(), mod_int(0))) + 1), cpp_char("\n"));
    return 0;
  }
  D.erase(remove(D.begin(), D.end(), 0), D.end());
  var sorted = [0];
  for (var d in D)
  {
    sorted.push_back((sorted.back() + d));
  }
  sort(sorted.begin(), sorted.end());
  sorted.erase(unique(sorted.begin(), sorted.end()), sorted.end());
  var ON = N;
  N = int_cpp(sorted.size());
  var sequence = [0];
  var get_index = __cpp_lambda_7;
  for (var d in D)
  {
    var it = lower_bound(sorted.begin(), sorted.end(), sequence.back());
    var target = (sequence.back() + d);
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
  var S = int_cpp(sequence.size());
  var longest = 0;
  {
    var i = 0;
    while ((i < S))
    {
      {
        var j = i;
        while ((j < S))
        {
          longest = max(longest, (sequence[j] - sequence[i]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var choose_loop = __cpp_lambda_8;
  var total = 0;
  {
    var initial = 0;
    while ((initial < N))
    {
      var goal = get_index((sorted[initial] + longest));
      if (((goal >= N) || (sorted[goal] != (sorted[initial] + longest))))
      {
        initial += 1;
        continue;
      }
      var ready_for = cpp_construct(N, 0);
      var dp = cpp_construct((N + 1), vector((ON + 1), vector((ON + 1), 0)));
      ready_for[initial] = 1;
      var recompute_ready_for = __cpp_lambda_9;
      {
        var i = 0;
        while ((i < (S - 1)))
        {
          if ((sequence[i] == sorted[goal]))
          {
            total += ready_for[goal];
          }
          var smaller = min(sequence[i], sequence[(i + 1)]);
          var bigger = max(sequence[i], sequence[(i + 1)]);
          var s_index = get_index(smaller);
          var b_index = get_index(bigger);
          assert((b_index == (s_index + 1)));
          if ((sequence[i] < sequence[(i + 1)]))
          {
            {
              var p = (ON - 1);
              while ((p >= 0))
              {
                {
                  var n = ((ON - 1) - p);
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
              var p = (ON - 1);
              while ((p >= 0))
              {
                {
                  var n = ((ON - 1) - p);
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

func __cpp_lambda_7(value: dynamic)
{
  return int_cpp((lower_bound(sorted.begin(), sorted.end(), value) - sorted.begin()));
}

func __cpp_lambda_8(n: dynamic, r: dynamic)
{
  if (((r < 0) || (r > n)))
  {
    return 0;
  }
  var product = 1;
  {
    var i = 0;
    while ((i < r))
    {
      product *= (n - i);
      i += 1;
    }
  }
  product *= inv_factorial(int_cpp(r));
  return product;
}

func __cpp_lambda_9(index: dynamic)
{
  if (((index <= initial) || (index > goal)))
  {
    return;
  }
  ready_for[index] = 0;
  var diff = (sorted[index] - sorted[(index - 1)]);
  {
    var p = 0;
    while ((p <= ON))
    {
      {
        var n = 0;
        while (((p + n) <= ON))
        {
          ready_for[index] += (dp[(index - 1)][p][n] * (if ((p == 0)) (if ((n == diff)) 1 else 0) else choose_loop(((diff - n) - 1), (p - 1))));
          n += 1;
        }
      }
      p += 1;
    }
  }
}
