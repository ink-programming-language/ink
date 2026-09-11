// Translated from solution.cpp.

var inf: dynamic = (LLONG_MAX / 2);

class node
{
  var ch: dynamic = cpp_uninitialized();
  var refcnt: dynamic = cpp_uninitialized();
  var len: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
  func node(c: dynamic, r: dynamic = 0, len: dynamic = -1) -> dynamic
  {
      self->ch = cpp_construct(c);
      self->refcnt = cpp_construct(r);
      self->len = cpp_construct(len);
    }
}

class state
{
  var stnum: dynamic = cpp_uninitialized();
  var nd: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var cntr: dynamic = cpp_uninitialized();
  func state(s: dynamic, nd: dynamic, a: dynamic, b: dynamic, c: dynamic) -> dynamic
  {
      self->stnum = cpp_construct(s);
      self->nd = cpp_construct(nd);
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
      self->cntr = cpp_construct(c);
    }
}

var alp: dynamic = cpp_uninitialized();

var emp: dynamic = cpp_construct(0, inf, 0);

var ans: dynamic = cpp_uninitialized();

func getnode(c: dynamic) -> dynamic
{
  if ((c == cpp_char(".")))
  {
    return (&emp);
  }
  var ret: dynamic = alp[(c - cpp_char("a"))];
  if ((!ret))
  {
    ret = cpp_new(c);
  }
  return ret;
}

func delnode(nd: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  q.push(nd);
  while (q.empty())
  {
    nd = q.front();
    q.pop();
    {
      var i: dynamic = 0;
      while ((i < nd->to.size()))
      {
        var next: dynamic = nd->to[i];
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

func dfs(nd0: dynamic, a0: dynamic, b0: dynamic) -> dynamic
{
  var stk: dynamic = cpp_uninitialized();
  stk.push(state(0, nd0, a0, b0, 0));
  while ((!stk.empty()))
  {
    var tp: dynamic = stk.top();
    var num: dynamic = tp.stnum;
    var nd: dynamic = tp.nd;
    var a: dynamic = tp.a;
    var b: dynamic = tp.b;
    var cntr: dynamic = tp.cntr;
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
      var next: dynamic = nd->to[cntr];
      while ((next->to.size() == 1))
      {
        var tmp: dynamic = next;
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
      var next: dynamic = nd->to[cntr];
      var olen: dynamic = next->len;
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var s: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(s, q, a, b);
  a -= 1;
  var root: dynamic = cpp_new(0);
  root->to.resize(s.size());
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      var next: dynamic = getnode(s[i]);
      root->to[i] = next;
      next->refcnt += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      read(c, s);
      var nd: dynamic = alp[(c - cpp_char("a"))];
      if ((!nd))
      {
        i += 1;
        continue;
      }
      alp[(c - cpp_char("a"))] = 0;
      nd->ch = 0;
      nd->to.resize(s.size());
      {
        var j: dynamic = 0;
        while ((j < s.size()))
        {
          var next: dynamic = getnode(s[j]);
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
