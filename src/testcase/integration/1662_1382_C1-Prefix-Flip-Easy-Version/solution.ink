// Translated from solution.cpp.

var MOD = (1e9 + 7);

var MOD1 = (1e9 + 123);

var MOD2 = (1e9 + 321);

func fun(s: dynamic)
{
  var i: dynamic;
  var v: dynamic;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic;
  var a: dynamic;
  var n: dynamic;
  var q: dynamic;
  var k: dynamic;
  var i: dynamic;
  var l: dynamic;
  var m: dynamic;
  var c: dynamic;
  var u: dynamic;
  var f: dynamic;
  var j: dynamic;
  var p: dynamic;
  var r: dynamic;
  var x: dynamic;
  var y: dynamic;
  var s: dynamic;
  var b: dynamic;
  var d: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var S: dynamic;
    var P: dynamic;
    read(S, P);
    var v1: dynamic;
    var v2: dynamic;
    v1 = fun(S);
    v2 = fun(P);
    reverse(v2.begin(), v2.end());
    write((v1.size() + v2.size()), " ");
    for (var h in v1)
    {
      write(h, " ");
    }
    for (var h in v2)
    {
      write(h, " ");
    }
    write("\n");
  }
  return 0;
}
