// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0; i<n; ++i)");
}

class Tree
{
  var name: dynamic;
  var parent: dynamic;
  var child: dynamic;
  func Tree()
  {
      this->parent = cpp_construct(null);
    }
  func ~Tree()
  {
      REP(i, child.size());
      cpp_delete(child[i]);
    }
}

var tree: dynamic;

func find(t: dynamic, name: dynamic)
{
  if ((name == t->name))
  {
    return t;
  }
  REP(i, t->child.size());
  {
    var ret = find(t->child[i], name);
    if ((ret != null))
    {
      return ret;
    }
  }
  return null;
}

func isChild(t: dynamic, name: dynamic)
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

func isParent(t: dynamic, name: dynamic)
{
  if ((t->parent->name == name))
  {
    return true;
  }
  return false;
}

func isSibling(t: dynamic, name: dynamic)
{
  var p = t->parent;
  return isChild(p, name);
}

func isDescendant(t: dynamic, name: dynamic)
{
  var ret = find(t, name);
  if ((ret == null))
  {
    return false;
  }
  return true;
}

func isAncestor(t: dynamic, name: dynamic)
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

func main()
{
  var n: dynamic;
  var q: dynamic;
  while (cpp_comma(((cin >> n) >> q), ((n || q))))
  {
    tree = cpp_new();
    var now = tree;
    var prev = tree;
    var sp = -1;
    cin.ignore();
    write("\n");
    cpp_delete(tree);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var str: dynamic;
      getline(cin, str);
      var cnt = 0;
      REP(j, str.size());
      if ((str[j] == cpp_char(" ")))
      {
        cnt += 1;
      }
      var name = cpp_construct((str.begin() + cnt), str.end());
      REP(j, (((sp - cnt)) + 1)) = now->parent;
      var nw = cpp_new();
      nw->parent = now;
      nw->name = name;
      now->child.push_back(nw);
      now = nw;
      sp = cnt;
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var ret: dynamic;
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      read(a, b, b, b, c, c);
      c = c.substr(0, (c.size() - 1));
      var s = find(tree, c);
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
        write((if (ret) "True" else "False"), "\n");
      }
    }
