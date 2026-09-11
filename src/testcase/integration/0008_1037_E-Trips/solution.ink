// Translated from solution.cpp.

var module: dynamic = 1000000007;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      a -= 1;
      b -= 1;
      nbr[a] += 1;
      nbr[b] += 1;
      to[a].insert(b);
      to[b].insert(a);
      edges[i] = [a, b];
      i += 1;
    }
  }
  var all: dynamic = n;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((!vis[i]))
      {
        if ((nbr[i] < k))
        {
          var q: dynamic = cpp_uninitialized();
          q.push(i);
          vis[i] = true;
          all -= 1;
          while ((!q.empty()))
          {
            var cur: dynamic = q.front();
            q.pop();
            for (var t: dynamic in to[cur])
            {
              nbr[t] -= 1;
              to[t].erase(cur);
              if (((nbr[t] < k) && (!vis[t])))
              {
                q.push(t);
                vis[t] = true;
                all -= 1;
              }
            }
          }
        }
      }
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  var answer: dynamic = cpp_uninitialized();
  answer.push_back(all);
  {
    var i: dynamic = (m - 1);
    while ((i > 0))
    {
      var (a, b): dynamic = edges[i];
      if (((!vis[a]) && (!vis[b])))
      {
        to[a].erase(b);
        to[b].erase(a);
        nbr[a] -= 1;
        nbr[b] -= 1;
        if ((nbr[a] < k))
        {
          q.push(a);
          vis[a] = true;
          all -= 1;
        }
        if ((nbr[b] < k))
        {
          q.push(b);
          vis[b] = true;
          all -= 1;
        }
        while ((!q.empty()))
        {
          var cur: dynamic = q.front();
          q.pop();
          for (var t: dynamic in to[cur])
          {
            nbr[t] -= 1;
            to[t].erase(cur);
            if (((nbr[t] < k) && (!vis[t])))
            {
              q.push(t);
              vis[t] = true;
              all -= 1;
            }
          }
        }
      }
      answer.push_back(all);
      i -= 1;
    }
  }
  {
    var i: dynamic = (answer.size() - 1);
    while ((i >= 0))
    {
      write(answer[i], "\n");
      i -= 1;
    }
  }
}
