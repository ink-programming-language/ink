// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func rer(i: dynamic, l: dynamic, u: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=(int)(l);(i)<=(int)(u);++(i))");
}

func reu(i: dynamic, l: dynamic, u: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=(int)(l);(i)<(int)(u);++(i))");
}

var __cpp_top_level_1: dynamic = cpp_fragment("defined(_MSC_VER) || __cplusplus > 199711L");

func aut(r: dynamic, v: dynamic) -> dynamic
{
  return cpp_expression("#include <st");
}

func aut(r: dynamic, v: dynamic) -> dynamic
{
  return cpp_expression("#include <string> #");
}

func each(it: dynamic, o: dynamic) -> dynamic
{
  cpp_macro("for(aut(it, (o).begin()); it != (o).end(); ++ it)");
}

func all(o: dynamic) -> dynamic
{
  return cpp_expression("#include <string> #inc");
}

func pb(x: dynamic) -> dynamic
{
  return cpp_expression("#include <st");
}

func mp(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <string>");
}

func mset(m: dynamic, v: dynamic) -> dynamic
{
  return cpp_expression("#include <string> #in");
}

var INF: dynamic = cpp_expression("#include <");

var INFL: dynamic = cpp_expression("#include <string> #i");

func amin(x: dynamic, y: dynamic) -> dynamic
{
  if ((y < x))
  {
    x = y;
  }
}

func amax(x: dynamic, y: dynamic) -> dynamic
{
  if ((x < y))
  {
    x = y;
  }
}

class Partition
{
  func getGroundSize() -> dynamic
  {
      return cpp_cast(name.size());
    }
  func getNumberOfSets() -> dynamic
  {
      return cpp_cast(head.size());
    }
  func getSize(i: dynamic) -> dynamic
  {
      return size[i];
    }
  func getName(i: dynamic) -> dynamic
  {
      return name[i];
    }
  func getList(i: dynamic) -> dynamic
  {
      return List(head[i], next);
    }
  func init(n: dynamic) -> dynamic
  {
      head.assign(1, 0);
      size.assign(1, n);
      next.resize(n);
      prev.resize(n);
      name.assign(n, 0);
      head[0] = 0;
    }
  func addEmptySet() -> dynamic
  {
      var i: dynamic = cpp_cast(head.size());
      head.push_back(-1);
      size.push_back(0);
      return i;
    }
  func moveElement(element: dynamic, id: dynamic) -> dynamic
  {
      remove(element);
      insert(element, id);
    }
  var head: dynamic = cpp_uninitialized();
  var size: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var prev: dynamic = cpp_uninitialized();
  var name: dynamic = cpp_uninitialized();
  func remove(a: dynamic) -> dynamic
  {
      var p: dynamic = prev[a];
      var n: dynamic = next[a];
      var x: dynamic = name[a];
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
  func insert(a: dynamic, x: dynamic) -> dynamic
  {
      var n: dynamic = head[x];
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
  var state: dynamic = cpp_uninitialized();
  var letter: dynamic = cpp_uninitialized();
  func Transition() -> dynamic
  {
    }
  func Transition(state: dynamic, letter: dynamic) -> dynamic
  {
      self->state = cpp_construct(state);
      self->letter = cpp_construct(letter);
    }
}

class TransitionMap
{
  var Alphas: dynamic = cpp_uninitialized();
  var AlphaBase: dynamic = cpp_uninitialized();
  var vec: dynamic = cpp_uninitialized();
  func TransitionMap(n: dynamic) -> dynamic
  {
      self->vec = cpp_construct((n * Alphas));
    }
  func operator_index(t: dynamic) -> dynamic
  {
      return vec[((t.state * Alphas) + ((t.letter - AlphaBase)))];
    }
  func get(t: dynamic) -> dynamic
  {
      return vec[((t.state * Alphas) + ((t.letter - AlphaBase)))];
    }
}

func stabilize(graph: dynamic, partition: dynamic) -> dynamic
{
  var letters: dynamic = [cpp_char("0"), cpp_char("1")];
  var n: dynamic = cpp_cast(graph.size());
  var maxPartitions: dynamic = max(n, 2);
  assert((partition.getGroundSize() == n));
  assert((partition.getNumberOfSets() == 2));
  assert(((partition.getSize(0) + partition.getSize(1)) == n));
  rep(i, n);
  for (var e: dynamic in graph[i])
  {
    var t: dynamic = cpp_construct(e.state, e.letter);
    var u: dynamic = cpp_construct(partition.getName(e.state), e.letter);
    invGraph[t].emplace_back(i);
  }
  var que: dynamic = cpp_uninitialized();
  var firstSet: dynamic =  ((partition.getSize(0) < partition.getSize(1))) ? 0 : 1;
  que.push_back(firstSet);
  onQueue[firstSet] = true;
  var A: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_uninitialized();
  var Ys: dynamic = cpp_uninitialized();
  {
    var qh: dynamic = 0;
    while ((qh != que.size()))
    {
      var setA: dynamic = que[cpp_update(qh, "++")];
      onQueue[setA] = false;
      A.clear();
      for (var a: dynamic in partition.getList(setA))
      {
        A.push_back(a);
      }
      for (var c: dynamic in letters)
      {
        X.clear();
        for (var p: dynamic in A)
        {
          for (var x: dynamic in invGraph.get(Transition(p, c)))
          {
            X.push_back(x);
          }
        }
        Ys.clear();
        for (var x: dynamic in X)
        {
          var setY: dynamic = partition.getName(x);
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
        for (var setY: dynamic in Ys)
        {
          var YSize: dynamic = partition.getSize(setY);
          var intersectionSize: dynamic = cpp_cast(intersection[setY].size());
          if ((intersectionSize == YSize))
          {
            continue;
          }
          var newSet: dynamic = partition.addEmptySet();
          for (var y: dynamic in intersection[setY])
          {
            partition.moveElement(y, newSet);
          }
          if (onQueue[setY])
          {
            que.push_back(newSet);
            onQueue[newSet] = true;
          } else
          {
            var smallSet: dynamic =  (((YSize - intersectionSize) < intersectionSize)) ? setY : newSet;
            que.push_back(smallSet);
            onQueue[smallSet] = true;
          }
        }
        for (var setY: dynamic in Ys)
        {
          setVisited[setY] = false;
          for (var y: dynamic in intersection[setY])
          {
            stateVisited[y] = false;
          }
          intersection[setY].clear();
        }
      }
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while ((~scanf("%d%d", (&n), (&m))))
  {
    var partition: dynamic = cpp_uninitialized();
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      next[i] =  ((i == (n - 1))) ? -1 : (i + 1);
      prev[i] = (i - 1);
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var v: dynamic = cpp_uninitialized();
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&v), (&s), (&t));
      initpart[i] = v;
      graph[i].emplace_back(s, cpp_char("0"));
      graph[i].emplace_back(t, cpp_char("1"));
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var q: dynamic = cpp_uninitialized();
      scanf("%d", (&q));
      var ans: dynamic = partition.getSize(partition.getName(q));
      printf("%d\n", ans);
    }
