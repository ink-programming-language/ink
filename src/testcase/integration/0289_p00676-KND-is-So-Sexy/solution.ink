// Translated from solution.cpp.

var DBG = cpp_expression("#");

func dump(o: dynamic)
{
  cpp_macro("if(DBG){cerr<<#o<<\" \"<<(o)<<\" \";}");
}

func dumpl(o: dynamic)
{
  cpp_macro("if(DBG){cerr<<#o<<\" \"<<(o)<<endl;}");
}

func dumpc(o: dynamic)
{
  cpp_macro("if(DBG){cerr<<#o; for(auto &e:(o))cerr<<\" \"<<e;cerr<<endl;}");
}

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);i++)");
}

func rrep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(b)-1;i>=(a);i--)");
}

func all(c: dynamic)
{
  return cpp_expression("#include \"bits/");
}

var INF = if ((cpp_sizeof(dynamic) == cpp_sizeof(dynamic))) 0x3f3f3f3f3f3f3f3f else 0x3f3f3f3f;

var MOD = cpp_cast(((1e9 + 7)));

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func area(a: dynamic, b: dynamic, c: dynamic)
{
  var s = ((((a + b) + c)) / 2);
  return sqrt((((s * ((s - a))) * ((s - b))) * ((s - c))));
}

func main()
{
  write(fixed, setprecision(8));
  {
    var a: dynamic;
    var l: dynamic;
    var x: dynamic;
    while (((((cin >> a) >> l) >> x) && l))
    {
      write((area(a, l, l) + (area(l, (((l + x)) / 2), (((l + x)) / 2)) * 2)), "\n");
    }
  }
  return 0;
}
