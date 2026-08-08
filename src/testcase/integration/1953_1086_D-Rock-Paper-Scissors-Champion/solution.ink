// Translated from solution.cpp.

func mod(n: dynamic, m: dynamic)
{
  var ret = (n % m);
  if ((ret < 0))
  {
    ret += m;
  }
  return ret;
}

func gcd(a: dynamic, b: dynamic)
{
  return (if ((b == 0)) a else gcd(b, (a % b)));
}

func exp(a: dynamic, b: dynamic, m: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return mod(a, m);
  }
  var k = mod(exp(a, (b / 2), m), m);
  if ((b & 1))
  {
    return mod((a * mod((k * k), m)), m);
  } else
  {
    return mod((k * k), m);
  }
}

class Bit
{
  var BIT: dynamic;
  var N: dynamic;
  func Bit()
  {
    }
  func Bit(n: dynamic)
  {
      BIT.resize((n + 100), 0);
      N = (n + 10);
    }
  func update(x: dynamic, v: dynamic)
  {
      while ((x < N))
      {
        BIT[x] += v;
        x += ((x & (-x)));
      }
    }
  func sum(x: dynamic)
  {
      var r = 0;
      while ((x > 0))
      {
        r += BIT[x];
        x -= ((x & (-x)));
      }
      return r;
    }
  func query(l: dynamic, r: dynamic)
  {
      return (sum(r) - sum((l - 1)));
    }
}

var mat = cpp_array(200100);

var S = cpp_array(3);

var BIT: dynamic;

func upd(pos: dynamic, c: dynamic, f: dynamic)
{
  if (f)
  {
    BIT[c].update(pos, 1);
    S[c].insert(pos);
  } else
  {
    var last = mat[pos];
    BIT[last].update(pos, -1);
    S[last].erase(pos);
    mat[pos] = c;
    BIT[c].update(pos, 1);
    S[c].insert(pos);
  }
}

var n: dynamic;

var q: dynamic;

func ask()
{
  {
    var i = 0;
    while ((i < 3))
    {
      if ((S[i].size() == 0))
      {
        if ((i == 2))
        {
          write(S[1].size(), "\n");
          return;
        }
        if ((i == 0))
        {
          if ((S[2].size() == 0))
          {
            write(S[1].size(), "\n");
          } else
          {
            write(S[2].size(), "\n");
          }
          return;
        }
        if ((i == 1))
        {
          write(S[0].size(), "\n");
          return;
        }
        return;
      }
      i += 1;
    }
  }
  var res = n;
  {
    var i = 0;
    while ((i < 3))
    {
      if ((i == 0))
      {
        var l = (*S[1].begin());
        var r = (*S[2].begin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
        r = (*S[1].rbegin());
        l = (*S[2].rbegin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
      }
      if ((i == 1))
      {
        var l = (*S[2].begin());
        var r = (*S[0].begin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
        r = (*S[2].rbegin());
        l = (*S[0].rbegin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
      }
      if ((i == 2))
      {
        var l = (*S[0].begin());
        var r = (*S[1].begin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
        r = (*S[0].rbegin());
        l = (*S[1].rbegin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
      }
      i += 1;
    }
  }
  write(res, "\n");
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, q);
  var mp: dynamic;
  mp[cpp_char("P")] = 0;
  mp[cpp_char("S")] = 1;
  mp[cpp_char("R")] = 2;
  BIT = vector(3);
  {
    var i = 0;
    while ((i < 3))
    {
      BIT[i] = Bit(n);
      {
        var j = 0;
        while ((j < (n + 10)))
        {
          BIT[i].BIT[j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var cc: dynamic;
      read(cc);
      mat[i] = mp[cc];
      upd(i, mat[i], 1);
      i += 1;
    }
  }
  ask();
  while (cpp_update(q, "--"))
  {
    var p: dynamic;
    var cc: dynamic;
    read(p, cc);
    var c = mp[cc];
    upd(p, c, 0);
    ask();
  }
}
