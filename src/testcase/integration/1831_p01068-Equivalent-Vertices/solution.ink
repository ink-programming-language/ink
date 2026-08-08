// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func rer(i: dynamic, l: dynamic, u: dynamic)
{
  cpp_macro("for(int (i)=(int)(l);(i)<=(int)(u);++(i))");
}

func reu(i: dynamic, l: dynamic, u: dynamic)
{
  cpp_macro("for(int (i)=(int)(l);(i)<(int)(u);++(i))");
}

var __cpp_top_level_1 = cpp_fragment("defined(_MSC_VER) || __cplusplus > 199711L");

func aut(r: dynamic, v: dynamic)
{
  return cpp_expression("#include <st");
}

func aut(r: dynamic, v: dynamic)
{
  return cpp_expression("#include <string> #");
}

func each(it: dynamic, o: dynamic)
{
  cpp_macro("for(aut(it, (o).begin()); it != (o).end(); ++ it)");
}

func all(o: dynamic)
{
  return cpp_expression("#include <string> #inc");
}

func pb(x: dynamic)
{
  return cpp_expression("#include <st");
}

func mp(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <string>");
}

func mset(m: dynamic, v: dynamic)
{
  return cpp_expression("#include <string> #in");
}

var INF = cpp_expression("#include <");

var INFL = cpp_expression("#include <string> #i");

func amin(x: dynamic, y: dynamic)
{
  if ((y < x))
  {
    x = y;
  }
}

func amax(x: dynamic, y: dynamic)
{
  if ((x < y))
  {
    x = y;
  }
}

class Partition
{
  func getGroundSize()
  {
      return cpp_cast(name.size());
    }
  func getNumberOfSets()
  {
      return cpp_cast(head.size());
    }
  func getSize(i: dynamic)
  {
      return size[i];
    }
  func getName(i: dynamic)
  {
      return name[i];
    }
  func getList(i: dynamic)
  {
      return List(head[i], next);
    }
  func init(n: dynamic)
  {
      head.assign(1, 0);
      size.assign(1, n);
      next.resize(n);
      prev.resize(n);
      name.assign(n, 0);
      head[0] = 0;
    }
  func addEmptySet()
  {
      var i = cpp_cast(head.size());
      head.push_back(-1);
      size.push_back(0);
      return i;
    }
  func moveElement(element: dynamic, id: dynamic)
  {
      remove(element);
      insert(element, id);
    }
  var head: dynamic;
  var size: dynamic;
  var next: dynamic;
  var prev: dynamic;
  var name: dynamic;
  func remove(a: dynamic)
  {
      var p = prev[a];
      var n = next[a];
      var x = name[a];
      if ((n != -1))
      {
        prev[n] = p;
      }
      if ((p != -1))
      {
        next[p] = n;
      } else
      {
        head[x] = n;
      }
      prev[a] = cpp_assign(next[a], "=", -1);
      size[x] -= 1;
      name[a] = -1;
    }
  func insert(a: dynamic, x: dynamic)
  {
      var n = head[x];
      if ((n != -1))
      {
        prev[n] = a;
      }
      head[x] = a;
      prev[a] = -1;
      next[a] = n;
      size[x] += 1;
      name[a] = x;
    }
}

class Transition
{
  var state: dynamic;
  var letter: dynamic;
  func Transition()
  {
    }
  func Transition(state: dynamic, letter: dynamic)
  {
      this->state = cpp_construct(state);
      this->letter = cpp_construct(letter);
    }
}

class TransitionMap
{
  var Alphas: dynamic;
  var AlphaBase: dynamic;
  var vec: dynamic;
  func TransitionMap(n: dynamic)
  {
      this->vec = cpp_construct((n * Alphas));
    }
  func operator_index(t: dynamic)
  {
      return vec[((t.state * Alphas) + ((t.letter - AlphaBase)))];
    }
  func get(t: dynamic)
  {
      return vec[((t.state * Alphas) + ((t.letter - AlphaBase)))];
    }
}

func stabilize(graph: dynamic, partition: dynamic)
{
  var letters = [cpp_char("0"), cpp_char("1")];
  var n = cpp_cast(graph.size());
  var maxPartitions = max(n, 2);
  assert((partition.getGroundSize() == n));
  assert((partition.getNumberOfSets() == 2));
  assert(((partition.getSize(0) + partition.getSize(1)) == n));
  rep(i, n);
  for (var e in graph[i])
  {
    var t = cpp_construct(e.state, e.letter);
    var u = cpp_construct(partition.getName(e.state), e.letter);
    invGraph[t].emplace_back(i);
  }
  var que: dynamic;
  var firstSet = if ((partition.getSize(0) < partition.getSize(1))) 0 else 1;
  que.push_back(firstSet);
  onQueue[firstSet] = true;
  var A: dynamic;
  var X: dynamic;
  var Ys: dynamic;
  {
    var qh = 0;
    while ((qh != que.size()))
    {
      var setA = que[cpp_update(qh, "++")];
      onQueue[setA] = false;
      A.clear();
      for (var a in partition.getList(setA))
      {
        A.push_back(a);
      }
      for (var c in letters)
      {
        X.clear();
        for (var p in A)
        {
          for (var x in invGraph.get(Transition(p, c)))
          {
            X.push_back(x);
          }
        }
        Ys.clear();
        for (var x in X)
        {
          var setY = partition.getName(x);
          if ((!setVisited[setY]))
          {
            Ys.push_back(setY);
            setVisited[setY] = true;
          }
          if ((!stateVisited[x]))
          {
            intersection[setY].push_back(x);
            stateVisited[x] = true;
          }
        }
        for (var setY in Ys)
        {
          var YSize = partition.getSize(setY);
          var intersectionSize = cpp_cast(intersection[setY].size());
          if ((intersectionSize == YSize))
          {
            continue;
          }
          var newSet = partition.addEmptySet();
          for (var y in intersection[setY])
          {
            partition.moveElement(y, newSet);
          }
          if (onQueue[setY])
          {
            que.push_back(newSet);
            onQueue[newSet] = true;
          } else
          {
            var smallSet = if (((YSize - intersectionSize) < intersectionSize)) setY else newSet;
            que.push_back(smallSet);
            onQueue[smallSet] = true;
          }
        }
        for (var setY in Ys)
        {
          setVisited[setY] = false;
          for (var y in intersection[setY])
          {
            stateVisited[y] = false;
          }
          intersection[setY].clear();
        }
      }
    }
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  while ((~scanf("%d%d", (&n), (&m))))
  {
    var partition: dynamic;
    partition.init(n);
    partition.addEmptySet();
    rep(i, n);
    if ((initpart[i] == 1))
    {
      partition.moveElement(i, 1);
    }
    stabilize(graph, partition);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      next[i] = if ((i == (n - 1))) -1 else (i + 1);
      prev[i] = (i - 1);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var v: dynamic;
      var s: dynamic;
      var t: dynamic;
      scanf("%d%d%d", (&v), (&s), (&t));
      initpart[i] = v;
      graph[i].emplace_back(s, cpp_char("0"));
      graph[i].emplace_back(t, cpp_char("1"));
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var q: dynamic;
      scanf("%d", (&q));
      var ans = partition.getSize(partition.getName(q));
      printf("%d\n", ans);
    }
