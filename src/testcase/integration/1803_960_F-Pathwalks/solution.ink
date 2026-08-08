// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var i: dynamic;

var x: dynamic;

var y: dynamic;

var w: dynamic;

var ans: dynamic;

var nod = cpp_array(100005);

func query(w: dynamic, m: dynamic)
{
  var it = m.lower_bound(w);
  if ((it == m.begin()))
  {
    return 0;
  }
  it = prev(it);
  return it->second;
}

func upd(val: dynamic, w: dynamic, m: dynamic)
{
  var it = m.upper_bound(w);
  if ((it != m.begin()))
  {
    it = prev(it);
    if ((val <= it->second))
    {
      return;
    }
  }
  while (true)
  {
    var it = m.upper_bound(w);
    if ((it == m.end()))
    {
      break;
    }
    if ((it->second > val))
    {
      break;
    }
    m.erase(it);
  }
  m[w] = val;
}

func main()
{
  read(n, m);
  {
    i = 1;
    while ((i <= m))
    {
      read(x, y, w);
      var len = query(w, nod[x]);
      upd((len + 1), w, nod[y]);
      ans = max(ans, (len + 1));
      i += 1;
    }
  }
  write(ans);
  return 0;
}
