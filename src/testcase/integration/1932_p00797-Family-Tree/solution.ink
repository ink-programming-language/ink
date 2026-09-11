// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0; i<n; ++i)");
}

class Tree
{
  var name: dynamic = cpp_uninitialized();
  var parent: dynamic = cpp_uninitialized();
  var child: dynamic = cpp_uninitialized();
  func Tree() -> dynamic
  {
      self->parent = cpp_construct(null);
    }
  func cpp_destruct_Tree() -> dynamic
  {
      REP(i, child.size());
      cpp_delete(child[i]);
    }
}

var tree: dynamic = cpp_uninitialized();

func find(t: dynamic, name: dynamic) -> dynamic
{
  if ((name == t->name))
  {
    return t;
  }
  REP(i, t->child.size());
  {
    var ret: dynamic = find(t->child[i], name);
    if ((ret != null))
    {
      return ret;
    }
  }
  return null;
}

func isChild(t: dynamic, name: dynamic) -> dynamic
{
  REP(i, t->child.size());
  {
    if ((t->child[i]->name == name))
    {
      return true;
    }
  }
  return false;
}

func isParent(t: dynamic, name: dynamic) -> dynamic
{
  if ((t->parent->name == name))
  {
    return true;
  }
  return false;
}

func isSibling(t: dynamic, name: dynamic) -> dynamic
{
  var p: dynamic = t->parent;
  return isChild(p, name);
}

func isDescendant(t: dynamic, name: dynamic) -> dynamic
{
  var ret: dynamic = find(t, name);
  if ((ret == null))
  {
    return false;
  }
  return true;
}

func isAncestor(t: dynamic, name: dynamic) -> dynamic
{
  if ((t->name == name))
  {
    return true;
  }
  if ((t->parent == null))
  {
    return false;
  }
  return isAncestor(t->parent, name);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> n) >> q), ((n || q))))
  {
    tree = cpp_new();
    var now: dynamic = tree;
    var prev: dynamic = tree;
    var sp: dynamic = -1;
    cin.ignore();
    write("\n");
    cpp_delete(tree);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var str: dynamic = cpp_uninitialized();
      getline(cin, str);
      var cnt: dynamic = 0;
      REP(j, str.size());
      if ((str[j] == cpp_char(" ")))
      {
        cnt += 1;
      }
      var name: dynamic = cpp_construct((str.begin() + cnt), str.end());
      REP(j, (((sp - cnt)) + 1)) = now->parent;
      var nw: dynamic = cpp_new();
      nw->parent = now;
      nw->name = name;
      now->child.push_back(nw);
      now = nw;
      sp = cnt;
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var ret: dynamic = cpp_uninitialized();
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(a, b, b, b, c, c);
      c = c.substr(0, (c.size() - 1));
      var s: dynamic = find(tree, c);
      if ((s == null))
      {
        write("False", "\n");
      } else
      {
        if ((b[0] == cpp_char("c")))
        {
          ret = isChild(s, a);
        }
        if ((b[0] == cpp_char("p")))
        {
          ret = isParent(s, a);
        }
        if ((b[0] == cpp_char("s")))
        {
          ret = isSibling(s, a);
        }
        if ((b[0] == cpp_char("d")))
        {
          ret = isDescendant(s, a);
        }
        if ((b[0] == cpp_char("a")))
        {
          ret = isAncestor(s, a);
        }
        write(( (ret) ? "True" : "False"), "\n");
      }
    }
