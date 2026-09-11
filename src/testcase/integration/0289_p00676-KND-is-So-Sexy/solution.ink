// Translated from solution.cpp.

var DBG: dynamic = cpp_expression("#");

func dump(o: dynamic) -> dynamic
{
  cpp_macro("if(DBG){cerr<<#o<<\" \"<<(o)<<\" \";}");
}

func dumpl(o: dynamic) -> dynamic
{
  cpp_macro("if(DBG){cerr<<#o<<\" \"<<(o)<<endl;}");
}

func dumpc(o: dynamic) -> dynamic
{
  cpp_macro("if(DBG){cerr<<#o; for(auto &e:(o))cerr<<\" \"<<e;cerr<<endl;}");
}

func rep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);i++)");
}

func rrep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(b)-1;i>=(a);i--)");
}

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include \"bits/");
}

var INF: dynamic =  ((cpp_sizeof(dynamic) == cpp_sizeof(dynamic))) ? 0x3f3f3f3f3f3f3f3f : 0x3f3f3f3f;

var MOD: dynamic = cpp_cast(((1e9 + 7)));

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func area(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var s: dynamic = ((((a + b) + c)) / 2);
  return sqrt((((s * ((s - a))) * ((s - b))) * ((s - c))));
}

func main() -> dynamic
{
  write(fixed, setprecision(8));
  {
    var a: dynamic = cpp_uninitialized();
    var l: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    while (((((cin >> a) >> l) >> x) && l))
    {
      write((area(a, l, l) + (area(l, (((l + x)) / 2), (((l + x)) / 2)) * 2)), "\n");
    }
  }
  return 0;
}
