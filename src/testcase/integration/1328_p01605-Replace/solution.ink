// Translated from solution.cpp.

var inf = (LLONG_MAX / 2);

class node
{
  var ch: dynamic;
  var refcnt: dynamic;
  var len: dynamic;
  var to: dynamic;
  func node(c: dynamic, r: dynamic = 0, len: dynamic = -1)
  {
      this->ch = cpp_construct(c);
      this->refcnt = cpp_construct(r);
      this->len = cpp_construct(len);
    }
}

class state
{
  var stnum: dynamic;
  var nd: dynamic;
  var a: dynamic;
  var b: dynamic;
  var cntr: dynamic;
  func state(s: dynamic, nd: dynamic, a: dynamic, b: dynamic, c: dynamic)
  {
      this->stnum = cpp_construct(s);
      this->nd = cpp_construct(nd);
      this->a = cpp_construct(a);
      this->b = cpp_construct(b);
      this->cntr = cpp_construct(c);
    }
}

var alp: dynamic;

var emp = cpp_construct(0, inf, 0);

var ans: dynamic;

func getnode(c: dynamic)
{
  if ((c == cpp_char(".")))
  {
    return (&emp);
  }
  var ret = alp[(c - cpp_char("a"))];
  if ((!ret))
  {
    ret = cpp_new(c);
  }
  return ret;
}

func delnode(nd: dynamic)
{
  var q: dynamic;
  q.push(nd);
  while (q.empty())
  {
    nd = q.front();
    q.pop();
    {
      var i = 0;
      while ((i < nd->to.size()))
      {
        var next = nd->to[i];
        if ((!cpp_update(next->refcnt, "--")))
        {
          q.push(next);
        }
        i += 1;
      }
    }
    cpp_delete(nd);
  }
}

func dfs(nd0: dynamic, a0: dynamic, b0: dynamic)
{
  var stk: dynamic;
  stk.push(state(0, nd0, a0, b0, 0));
  while ((!stk.empty()))
  {
    var tp = stk.top();
    var num = tp.stnum;
    var nd = tp.nd;
    var a = tp.a;
    var b = tp.b;
    var cntr = tp.cntr;
    if ((num == 0))
    {
      if ((a >= b))
      {
        stk.pop();
        continue;
      }
      if (nd->ch)
      {
        nd->len = 1;
        if ((a == 0))
        {
          ans += nd->ch;
        }
        stk.pop();
        continue;
      }
      if (((nd->len >= 0) && (nd->len <= a)))
      {
        stk.pop();
        continue;
      }
      nd->len = 0;
      num = 1;
    } else if ((num == 1))
    {
      if ((cntr >= nd->to.size()))
      {
        stk.pop();
        continue;
      }
      var next = nd->to[cntr];
      while ((next->to.size() == 1))
      {
        var tmp = next;
        nd->to[cntr] = next->to[0];
        next = next->to[0];
        if (cpp_update(tmp->refcnt, "--"))
        {
          next->refcnt += 1;
          break;
        }
        cpp_delete(tmp);
      }
      num = 2;
      stk.push(state(0, next, a, b, 0));
    } else if ((num == 2))
    {
      var next = nd->to[cntr];
      var olen = next->len;
      a = max((a - olen), 0);
      b = max((b - olen), 0);
      nd->len = min((nd->len + olen), inf);
      if ((next->len == 0))
      {
        nd->to.erase((nd->to.begin() + cntr));
        cntr -= 1;
        if ((!cpp_update(next->refcnt, "--")))
        {
          delnode(next);
        }
      }
      cntr += 1;
      num = 1;
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  var s: dynamic;
  var q: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  read(s, q, a, b);
  a -= 1;
  var root = cpp_new(0);
  root->to.resize(s.size());
  {
    var i = 0;
    while ((i < s.size()))
    {
      var next = getnode(s[i]);
      root->to[i] = next;
      next->refcnt += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      read(c, s);
      var nd = alp[(c - cpp_char("a"))];
      if ((!nd))
      {
        i += 1;
        continue;
      }
      alp[(c - cpp_char("a"))] = 0;
      nd->ch = 0;
      nd->to.resize(s.size());
      {
        var j = 0;
        while ((j < s.size()))
        {
          var next = getnode(s[j]);
          nd->to[j] = next;
          next->refcnt += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dfs(root, a, b);
  if ((ans.size() < (b - a)))
  {
    ans = ".";
  }
  write(ans, cpp_char("\n"));
  delnode(root);
}
