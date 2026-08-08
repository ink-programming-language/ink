// Translated from solution.cpp.

class TrieNode
{
  var postive: dynamic;
  var leaf: dynamic;
  var zero: dynamic;
  var one: dynamic;
  func TrieNode()
  {
      postive = cpp_assign(leaf, "=", 0);
      zero = cpp_assign(one, "=", null);
    }
}

func add(s: dynamic, idx: dynamic, postive: dynamic, cur: dynamic)
{
  cur->postive |= postive;
  if ((idx == s.size()))
  {
    cur->leaf = 1;
    return;
  }
  if ((s[idx] == cpp_char("1")))
  {
    if ((cur->one == null))
    {
      cur->one = cpp_new();
    }
    add(s, (idx + 1), postive, cur->one);
  } else
  {
    if ((cur->zero == null))
    {
      cur->zero = cpp_new();
    }
    add(s, (idx + 1), postive, cur->zero);
  }
  if (((((cur->one != null) && (cur->zero != null)) && cur->one->leaf) && cur->zero->leaf))
  {
    cur->leaf = 1;
  }
}

func search(s: dynamic, idx: dynamic, postive: dynamic, cur: dynamic)
{
  if (cur->leaf)
  {
    return 1;
  }
  if ((idx == s.size()))
  {
    return postive;
  }
  if ((s[idx] == cpp_char("1")))
  {
    if ((cur->one == null))
    {
      return 0;
    }
    return search(s, (idx + 1), postive, cur->one);
  } else
  {
    if ((cur->zero == null))
    {
      return 0;
    }
    return search(s, (idx + 1), postive, cur->zero);
  }
}

var ips: dynamic;

func solve(s: dynamic, cur: dynamic)
{
  if ((!cur->postive))
  {
    ips.push_back(s);
    return;
  }
  if ((cur->zero != null))
  {
    s += cpp_char("0");
    solve(s, cur->zero);
    s.pop_back();
  }
  if ((cur->one != null))
  {
    s += cpp_char("1");
    solve(s, cur->one);
    s.pop_back();
  }
}

func to_binary(num: dynamic)
{
  var ret: dynamic;
  while ((num != 0))
  {
    ret += (((num % 2)) + cpp_char("0"));
    num /= 2;
  }
  while ((ret.size() < 8))
  {
    ret += cpp_char("0");
  }
  reverse(ret.begin(), ret.end());
  return ret;
}

func convert(s: dynamic)
{
  var bs = -1;
  var sub: dynamic;
  var cur = 0;
  {
    var i = 1;
    while ((i < s.size()))
    {
      if ((s[i] == 47))
      {
        bs = i;
        break;
      }
      if ((s[i] == cpp_char(".")))
      {
        sub.push_back(cur);
        cur = 0;
      } else
      {
        cur *= 10;
        cur += ((s[i] - cpp_char("0")));
      }
      i += 1;
    }
  }
  sub.push_back(cur);
  var ret: dynamic;
  for (var x in sub)
  {
    ret += to_binary(x);
  }
  if ((bs != -1))
  {
    cur = 0;
    {
      var i = (bs + 1);
      while ((i < s.size()))
      {
        cur *= 10;
        cur += (s[i] - cpp_char("0"));
        i += 1;
      }
    }
    ret = ret.substr(0, cur);
  }
  return ret;
}

func to_number(s: dynamic)
{
  var mul = 1;
  var ret = 0;
  {
    var i = (s.size() - 1);
    while ((i >= 0))
    {
      ret += (mul * ((s[i] - cpp_char("0"))));
      mul *= 2;
      i -= 1;
    }
  }
  var ret2: dynamic;
  if ((ret == 0))
  {
    ret2 = "0";
  }
  while ((ret != 0))
  {
    ret2 += (((ret % 10)) + cpp_char("0"));
    ret /= 10;
  }
  reverse(ret2.begin(), ret2.end());
  return ret2;
}

func tostring(number: dynamic)
{
  if ((number == 0))
  {
    return "0";
  }
  var ret: dynamic;
  while ((number != 0))
  {
    ret += (((number % 10)) + cpp_char("0"));
    number /= 10;
  }
  reverse(ret.begin(), ret.end());
  return ret;
}

func to_ip()
{
  for (var s in ips)
  {
    var cur = s;
    var r = s.size();
    while ((cur.size() < 32))
    {
      cur += cpp_char("0");
    }
    s = "";
    {
      var i = 0;
      while ((i < 32))
      {
        var sub = cur.substr(i, 8);
        if (i)
        {
          s += cpp_char(".");
        }
        s += to_number(sub);
        i += 8;
      }
    }
    s += 47;
    s += tostring(r);
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var root = cpp_new();
  var black: dynamic;
  var white: dynamic;
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      var s: dynamic;
      read(s);
      if ((s[0] == cpp_char("-")))
      {
        black.push_back(s);
      } else
      {
        white.push_back(s);
      }
      i += 1;
    }
  }
  for (var x in black)
  {
    var cur = convert(x);
    if ((!search(cur, 0, 0, root)))
    {
      add(cur, 0, 0, root);
    }
  }
  for (var x in white)
  {
    var cur = convert(x);
    if (search(cur, 0, 1, root))
    {
      write(-1);
      return 0;
    }
  }
  for (var x in white)
  {
    var cur = convert(x);
    if ((!search(cur, 0, 1, root)))
    {
      add(cur, 0, 1, root);
    }
  }
  var s: dynamic;
  solve(s, root);
  to_ip();
  write(ips.size(), cpp_char("\n"));
  for (var x in ips)
  {
    write(x, cpp_char("\n"));
  }
}
