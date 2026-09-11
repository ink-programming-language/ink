// Translated from solution.cpp.

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var op: dynamic = cpp_uninitialized();

var mini: dynamic = cpp_uninitialized();

var mij: dynamic = cpp_uninitialized();

var ls: dynamic = cpp_uninitialized();

var ld: dynamic = cpp_uninitialized();

var ul: dynamic = cpp_uninitialized();

var timp: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var maxl: dynamic = cpp_uninitialized();

var rasp: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(1000005);

var flower: dynamic = cpp_array(1000005);

var viz: dynamic = cpp_array(1000005);

var v: dynamic = cpp_array(1000005);

var rez: dynamic = cpp_uninitialized();

func go(nod: dynamic, p: dynamic) -> dynamic
{
  if (viz[nod])
  {
    return;
  }
  viz[nod] = 1;
  {
    var i: dynamic = 0;
    while ((i < v[nod].size()))
    {
      var nxt: dynamic = v[nod][i];
      if ((nxt == p))
      {
        i += 1;
        continue;
      }
      go(nxt, nod);
      dp[nod] += dp[nxt];
      flower[nod] += flower[nxt];
      i += 1;
    }
  }
  var total: dynamic = (dp[nod] + flower[nod]);
  if ((nod == b))
  {
    rez += (total + dp[nod]);
  } else
  {
    rez += (total * 2);
  }
  {
    var i: dynamic = 0;
    while ((i < v[nod].size()))
    {
      var nxt: dynamic = v[nod][i];
      if ((nxt == p))
      {
        i += 1;
        continue;
      }
      if ((nod == b))
      {
        rez += ((((total - dp[nxt]) - flower[nxt])) * dp[nxt]);
      } else
      {
        rez += ((((total - dp[nxt]) - flower[nxt])) * ((dp[nxt] + flower[nxt])));
      }
      i += 1;
    }
  }
  dp[nod] += 1;
  if ((nod == a))
  {
    flower[nod] += dp[nod];
    dp[nod] = 0;
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n, a, b);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      read(c, d);
      v[c].push_back(d);
      v[d].push_back(c);
      i += 1;
    }
  }
  go(b, -1);
  write(rez);
  return 0;
}
