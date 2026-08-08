// Translated from solution.cpp.

var DUMPOUT = cpp_expression("//#d");

func dump()
{
  cpp_macro("DUMPOUT<<\"  \";DUMPOUT<<#__VA_ARGS__<<\" :[\"<<__LINE__<<\":\"<<__FUNCTION__<<\"]\"<<endl;DUMPOUT<<\"    \";dump_func(__VA_ARGS__)");
}

func operator_shift_left(o: dynamic, m: dynamic)
{
  (((((o << "{") << m.first) << ", ") << m.second) << "}");
  return o;
}

func operator_shift_left(o: dynamic, m: dynamic)
{
  if (m.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*m.begin()));
  {
    var itr = cpp_update(m.begin(), "++");
    while ((itr != m.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, m: dynamic)
{
  if (m.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*m.begin()));
  {
    var itr = cpp_update(m.begin(), "++");
    while ((itr != m.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  if (v.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << v.front());
  {
    var itr = cpp_update(v.begin(), "++");
    while ((itr != v.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  if (v.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << v.front());
  {
    var itr = cpp_update(v.begin(), "++");
    while ((itr != v.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, s: dynamic)
{
  if (s.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*(s.begin())));
  {
    var itr = cpp_update(s.begin(), "++");
    while ((itr != s.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, s: dynamic)
{
  if (s.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << (*(s.begin())));
  {
    var itr = cpp_update(s.begin(), "++");
    while ((itr != s.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_left(o: dynamic, s: dynamic)
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

func operator_shift_left(o: dynamic, l: dynamic)
{
  if (l.empty())
  {
    (o << "{ }");
    return o;
  }
  ((o << "{") << l.front());
  {
    var itr = cpp_update(l.begin(), "++");
    while ((itr != l.end()))
    {
      ((o << ", ") << (*itr));
      itr += 1;
    }
  }
  (o << "}");
  return o;
}

func operator_shift_right(is: dynamic, m: dynamic)
{
  ((is >> m.first) >> m.second);
  return is;
}

func operator_shift_right(is: dynamic, v: dynamic)
{
  {
    var t = 0;
    while ((t < v.size()))
    {
      (is >> v[t]);
      t += 1;
    }
  }
  return is;
}

func operator_shift_right(is: dynamic, v: dynamic)
{
  {
    var t = 0;
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
  func print(os: dynamic, v: dynamic)
  {
      ((os << get(v)) << ", ");
      tp.print(os, v);
    }
}

class tp_Ty_N_N
{
  func print(os: dynamic, v: dynamic)
  {
      (os << get(v));
    }
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  (os << "{");
  aux.tp.print(os, t);
  (os << "}");
  return os;
}

func Fill(array: dynamic, val: dynamic)
{
  fill(cpp_cast(array), cpp_cast(((array + N))), val);
}

func format(fmt: dynamic, args: dynamic...)
{
  var len = snprintf(null, 0, fmt.c_str(), cpp_expand(args));
  var buf = cpp_construct((len + 1));
  snprintf((&buf[0]), (len + 1), fmt.c_str(), cpp_expand(args));
  return string_cpp((&buf[0]), ((&buf[0]) + len));
}

func dump_func()
{
  (DUMPOUT << endl);
}

func dump_func(head: dynamic, tail: dynamic...)
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

var PI = cpp_expression("//#define NDEBUG #incl");

var EPS = cpp_expression("//#d");

func rep(t: dynamic, n: dynamic)
{
  cpp_macro("for(int t=0;t<(n);++t)");
}

func all(j: dynamic)
{
  return cpp_expression("//#define NDEBUG #incl");
}

func SZ(j: dynamic)
{
  return cpp_expression("//#define NDEBUG");
}

var fake = cpp_expression("//#de");

class Timer
{
  var t: dynamic;
  func Timer()
  {
    }
  func time()
  {
      return (rdtsc() / 2.8e9);
      var a: dynamic;
      var d: dynamic;
      cpp_expression("__asm__ volatile(\"rdtsc\" : \"=a\"(a), \"=d\"(d))");
      return ((((d << 32) | a)) / 2.8e9);
    }
  func measure()
  {
      t = (time() - t);
    }
  func elapsedMs()
  {
      return (((time() - t)) * 1000.0);
    }
}

var timer: dynamic;

class Xorshift
{
  var x: dynamic;
  func next_int()
  {
      x = (x ^ ((x << 7)));
      return cpp_assign(x, "=", (x ^ ((x >> 9))));
    }
  func next_int(mod: dynamic)
  {
      x = (x ^ ((x << 7)));
      x = (x ^ ((x >> 9)));
      return (x % mod);
    }
  func next_int(l: dynamic, r: dynamic)
  {
      x = (x ^ ((x << 7)));
      x = (x ^ ((x >> 9)));
      return ((x % (((r - l) + 1))) + l);
    }
  func next_double()
  {
      return (double(next_int()) / UINT_MAX);
    }
}

var rnd: dynamic;

func shuffle_vector(v: dynamic, rnd: dynamic)
{
  var n = v.size();
  {
    var i = (n - 1);
    while ((i >= 1))
    {
      var r = rnd.next_int(i);
      swap(v[i], v[r]);
      i -= 1;
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var x: dynamic;
  read(x);
  write(b, cpp_char("\n"));
  write((~b), cpp_char("\n"));
  write(((b << 1)), cpp_char("\n"));
  write(((b >> 1)), cpp_char("\n"));
  return 0;
}
