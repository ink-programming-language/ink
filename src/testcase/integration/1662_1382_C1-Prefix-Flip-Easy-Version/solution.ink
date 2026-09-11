// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var MOD1: dynamic = (1e9 + 123);

var MOD2: dynamic = (1e9 + 321);

func fun(s: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  {
    i = 0;
    while (((i + 1) < s.length()))
    {
      if ((s[i] != s[(i + 1)]))
      {
        v.push_back((i + 1));
      }
      i += 1;
    }
  }
  if ((s[(s.length() - 1)] == cpp_char("1")))
  {
    v.push_back(s.length());
  }
  return v;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var S: dynamic = cpp_uninitialized();
    var P: dynamic = cpp_uninitialized();
    read(S, P);
    var v1: dynamic = cpp_uninitialized();
    var v2: dynamic = cpp_uninitialized();
    v1 = fun(S);
    v2 = fun(P);
    reverse(v2.begin(), v2.end());
    write((v1.size() + v2.size()), " ");
    for (var h: dynamic in v1)
    {
      write(h, " ");
    }
    for (var h: dynamic in v2)
    {
      write(h, " ");
    }
    write("\n");
  }
  return 0;
}
