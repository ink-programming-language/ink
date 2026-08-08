// Translated from solution.cpp.

var i: dynamic;

var j: dynamic;

var n: dynamic;

var m: dynamic;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var op: dynamic;

var mini: dynamic;

var mij: dynamic;

var ls: dynamic;

var ld: dynamic;

var ul: dynamic;

var timp: dynamic;

var k: dynamic;

var maxl: dynamic;

var rasp: dynamic;

var dp = cpp_array(1000005);

var flower = cpp_array(1000005);

var viz = cpp_array(1000005);

var v = cpp_array(1000005);

var rez: dynamic;

func go(nod: dynamic, p: dynamic)
{
  if (viz[nod])
  {
    return;
  }
  viz[nod] = 1;
  {
    var i = 0;
    while ((i < v[nod].size()))
    {
      var nxt = v[nod][i];
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
  var total = (dp[nod] + flower[nod]);
  if ((nod == b))
  {
    rez += (total + dp[nod]);
  } else
  {
    rez += (total * 2);
  }
  {
    var i = 0;
    while ((i < v[nod].size()))
    {
      var nxt = v[nod][i];
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n, a, b);
  {
    var i = 1;
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
