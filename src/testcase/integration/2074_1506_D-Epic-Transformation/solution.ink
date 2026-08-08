// Translated from solution.cpp.

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

func to_string(v: dynamic)
{
  var first = true;
  var res = "{";
  {
    var i = 0;
    while ((i < static_cast(v.size())))
    {
      if ((!first))
      {
        res += ", ";
      }
      first = false;
      res += to_string(v[i]);
      i += 1;
    }
  }
  res += "}";
  return res;
}

func to_string(v: dynamic)
{
  var res = "";
  {
    var i = 0;
    while ((i < N))
    {
      res += static_cast((cpp_char("0") + v[i]));
      i += 1;
    }
  }
  return res;
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

func to_string(p: dynamic)
{
  return (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
}

func to_string(p: dynamic)
{
  return (((((("(" + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ")");
}

func to_string(p: dynamic)
{
  return (((((((("(" + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ", ") + to_string(get(p))) + ")");
}

func debug_out()
{
  write("   ");
}

func debug_out_nl()
{
  write("\n");
}

func debug_out(H: dynamic, T: dynamic...)
{
  write(" ", to_string(H));
  debug_out(cpp_expand(T));
}

func debug_out_nl(H: dynamic, T: dynamic...)
{
  write(" ", to_string(H));
  debug_out_nl(cpp_expand(T));
}

func dbg()
{
  return cpp_expression("// g++ -std=c++17 -DLOCAL a.cpp -o ex && ./ex >tst.out 2>&");
}

func nl()
{
  return cpp_expression("// g++ -std=c++17 -DLOCAL a.cpp -o ex && ./ex >tst.out 2>&1 #");
}

func dbg()
{
  return cpp_expression("//");
}

func nl()
{
  return cpp_expression("//");
}

var ll = dynamic;

var MOD = (1e9 + 7);

var N = (2e5 + 10);

var n: dynamic;

var k: dynamic;

var s: dynamic;

func solve(test: dynamic, cin: dynamic, cout: dynamic)
{
  read(n);
  var mp: dynamic;
  {
    var i = 1;
    var x: dynamic;
    while ((i <= n))
    {
      read(x);
      mp[x] += 1;
      i += 1;
    }
  }
  var s: dynamic;
  for (var it in mp)
  {
    s.insert(it.second);
  }
  var ans = n;
  while ((cpp_cast(s.size()) > 1))
  {
    var it = s.end();
    it = prev(it);
    var it2 = prev(it);
    ans -= 2;
    var v1 = (*it);
    var v2 = (*it2);
    s.erase(it);
    s.erase(it2);
    if (((v1 - 1) > 0))
    {
      s.insert((v1 - 1));
    }
    if (((v2 - 1) > 0))
    {
      s.insert((v2 - 1));
    }
  }
  write(ans, cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  var multiTest = true;
  var t: dynamic;
  if (multiTest)
  {
    read(t);
  } else
  {
    t = 1;
  }
  {
    var test = 1;
    while ((test <= t))
    {
      solve(test, cin, cout);
      test += 1;
    }
  }
  return 0;
}
