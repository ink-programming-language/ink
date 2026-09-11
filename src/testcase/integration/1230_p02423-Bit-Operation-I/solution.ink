// Translated from solution.cpp.

var DUMPOUT: dynamic = cpp_expression("//#d");

func dump() -> dynamic
{
  cpp_macro("DUMPOUT<<\"  \";DUMPOUT<<#__VA_ARGS__<<\" :[\"<<__LINE__<<\":\"<<__FUNCTION__<<\"]\"<<endl;DUMPOUT<<\"    \";dump_func(__VA_ARGS__)");
}

func operator_shift_left(o: dynamic, m: dynamic) -> dynamic
{
  (((((o << "{") << m.first) << ", ") << m.second) << "}");
  return o;
}

func operator_shift_left(o: dynamic, m: dynamic) -> dynamic
{
  if (m.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*m.begin()));
  {
    var itr: dynamic = cpp_update(m.begin(), "++");
    while ((itr != m.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, m: dynamic) -> dynamic
{
  if (m.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*m.begin()));
  {
    var itr: dynamic = cpp_update(m.begin(), "++");
    while ((itr != m.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  if (v.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << v.front());
  {
    var itr: dynamic = cpp_update(v.begin(), "++");
    while ((itr != v.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  if (v.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << v.front());
  {
    var itr: dynamic = cpp_update(v.begin(), "++");
    while ((itr != v.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, s: dynamic) -> dynamic
{
  if (s.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*(s.begin())));
  {
    var itr: dynamic = cpp_update(s.begin(), "++");
    while ((itr != s.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, s: dynamic) -> dynamic
{
  if (s.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*(s.begin())));
  {
    var itr: dynamic = cpp_update(s.begin(), "++");
    while ((itr != s.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, s: dynamic) -> dynamic
{
  if (s.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << t.top());
  t.pop();
  while ((!t.empty()))
  {
    ((o << ", ") << t.top());
    t.pop();
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, l: dynamic) -> dynamic
{
  if (l.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << l.front());
  {
    var itr: dynamic = cpp_update(l.begin(), "++");
    while ((itr != l.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_right(is: dynamic, m: dynamic) -> dynamic
{
  ((is >> m.first) >> m.second);
  return is;
}

func operator_shift_right(is: dynamic, v: dynamic) -> dynamic
{
  {
    var t: dynamic = 0;
    while ((t < v.size()))
    {
      (is >> v[t]);
      t += 1;
    }
  }
  return is;
}

func operator_shift_right(is: dynamic, v: dynamic) -> dynamic
{
  {
    var t: dynamic = 0;
    while ((t < v.size()))
    {
      (is >> v[t]);
      t += 1;
    }
  }
  return is;
}

class tp
{
  func print(os: dynamic, v: dynamic) -> dynamic
  {
      ((os << get(v)) << ", ");
      tp.print(os, v);
    }
}

class tp_Ty_N_N
{
  func print(os: dynamic, v: dynamic) -> dynamic
  {
      (os << get(v));
    }
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  (os << "{");
  aux.tp.print(os, t);
  (os << "}");
  return os;
}

func Fill(array: dynamic, val: dynamic) -> dynamic
{
  fill(cpp_cast(array), cpp_cast(((array + N))), val);
}

func format(fmt: dynamic, args: dynamic...) -> dynamic
{
  var len: dynamic = snprintf(null, 0, fmt.c_str(), cpp_expand(args));
  var buf: dynamic = cpp_construct((len + 1));
  snprintf((&buf[0]), (len + 1), fmt.c_str(), cpp_expand(args));
  return string_cpp((&buf[0]), ((&buf[0]) + len));
}

func dump_func() -> dynamic
{
  (DUMPOUT << endl);
}

func dump_func(head: dynamic, tail: dynamic...) -> dynamic
{
  (DUMPOUT << head);
  if ((cpp_sizeof(Tail) == 0))
  {
    (DUMPOUT << " ");
  } else
  {
    (DUMPOUT << ", ");
  }
  dump_func(cpp_expand(move(tail)));
}

var PI: dynamic = cpp_expression("//#define NDEBUG #incl");

var EPS: dynamic = cpp_expression("//#d");

func rep(t: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int t=0;t<(n);++t)");
}

func all(j: dynamic) -> dynamic
{
  return cpp_expression("//#define NDEBUG #incl");
}

func SZ(j: dynamic) -> dynamic
{
  return cpp_expression("//#define NDEBUG");
}

var fake: dynamic = cpp_expression("//#de");

class Timer
{
  var t: dynamic = cpp_uninitialized();
  func Timer() -> dynamic
  {
    }
  func time() -> dynamic
  {
      return (rdtsc() / 2.8e9);
      var a: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
      cpp_expression("__asm__ volatile(\"rdtsc\" : \"=a\"(a), \"=d\"(d))");
      return ((((d << 32) | a)) / 2.8e9);
    }
  func measure() -> dynamic
  {
      t = (time() - t);
    }
  func elapsedMs() -> dynamic
  {
      return (((time() - t)) * 1000.0);
    }
}

var timer: dynamic = cpp_uninitialized();

class Xorshift
{
  var x: dynamic = cpp_uninitialized();
  func next_int() -> dynamic
  {
      x = (x ^ ((x << 7)));
      return cpp_assign(x, "=", (x ^ ((x >> 9))));
    }
  func next_int(mod: dynamic) -> dynamic
  {
      x = (x ^ ((x << 7)));
      x = (x ^ ((x >> 9)));
      return (x % mod);
    }
  func next_int(l: dynamic, r: dynamic) -> dynamic
  {
      x = (x ^ ((x << 7)));
      x = (x ^ ((x >> 9)));
      return ((x % (((r - l) + 1))) + l);
    }
  func next_double() -> dynamic
  {
      return (cpp_double(next_int()) / UINT_MAX);
    }
}

var rnd: dynamic = cpp_uninitialized();

func shuffle_vector(v: dynamic, rnd: dynamic) -> dynamic
{
  var n: dynamic = v.size();
  {
    var i: dynamic = (n - 1);
    while ((i >= 1))
    {
      var r: dynamic = rnd.next_int(i);
      swap(v[i], v[r]);
      i -= 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var x: dynamic = cpp_uninitialized();
  read(x);
  write(b, cpp_char("\n"));
  write((~b), cpp_char("\n"));
  write(((b << 1)), cpp_char("\n"));
  write(((b >> 1)), cpp_char("\n"));
  return 0;
}
