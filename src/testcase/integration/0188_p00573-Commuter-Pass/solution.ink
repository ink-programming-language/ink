// Translated from solution.cpp.

var ll = dynamic;

var linf = 1e15;

class edge
{
  var to: dynamic;
  var cost: dynamic;
}

class node
{
  var from_cpp: dynamic;
  var cost: dynamic;
  func operator_less(n1: dynamic)
  {
      return (n1.cost < cost);
    }
}

var N: dynamic;

var M: dynamic;

var S: dynamic;

var T: dynamic;

var U: dynamic;

var V: dynamic;

var G = cpp_array(100010);

var DU = cpp_array(100010);

var DV = cpp_array(100010);

var DS = cpp_array(100010);

var dp = cpp_array(100010, 3);

var used = cpp_array(100010);

func dijk(s: dynamic, D: dynamic)
{
  fill(D, ((D + N) + 1), linf);
  var pq: dynamic;
  pq.push([s, 0]);
  D[s] = 0;
  while ((!pq.empty()))
  {
    var n1 = pq.top();
    pq.pop();
    if ((D[n1.from_cpp] < n1.cost))
    {
      continue;
    }
    for (var u in G[n1.from_cpp])
    {
      var cost = (u.cost + n1.cost);
      if ((cost < D[u.to]))
      {
        D[u.to] = cost;
        pq.push([u.to, cost]);
      }
    }
  }
}

func solve()
{
  dijk(U, DU);
  dijk(V, DV);
  dijk(S, DS);
  {
    var i = 1;
    while ((i <= N))
    {
      dp[0][i] = DU[i];
      dp[1][i] = DV[i];
      dp[2][i] = linf;
      i += 1;
    }
  }
  var pq: dynamic;
  pq.push([S, 0]);
  while ((!pq.empty()))
  {
    var n1 = pq.top();
    pq.pop();
    for (var u in G[n1.from_cpp])
    {
      var cost = (u.cost + n1.cost);
      if ((DS[u.to] < cost))
      {
        continue;
      }
      dp[0][u.to] = min(dp[0][u.to], dp[0][n1.from_cpp]);
      dp[1][u.to] = min(dp[1][u.to], dp[1][n1.from_cpp]);
      if ((!used[u.to]))
      {
        used[u.to] = true;
        pq.push([u.to, DS[u.to]]);
      }
    }
  }
  {
    var i = 1;
    while ((i <= N))
    {
      dp[2][i] = min((dp[0][i] + DV[i]), (dp[1][i] + DU[i]));
      i += 1;
    }
  }
  fill(used, ((used + N) + 1), false);
  pq.push([S, 0]);
  while ((!pq.empty()))
  {
    var n1 = pq.top();
    pq.pop();
    for (var u in G[n1.from_cpp])
    {
      var cost = (u.cost + n1.cost);
      if ((DS[u.to] < cost))
      {
        continue;
      }
      dp[2][u.to] = min(dp[2][u.to], dp[2][n1.from_cpp]);
      if ((!used[u.to]))
      {
        used[u.to] = true;
        pq.push([u.to, DS[u.to]]);
      }
    }
  }
  return min(dp[2][T], DU[V]);
}

func init()
{
  read(N, M, S, T, U, V);
  {
    var i = 0;
    while ((i < M))
    {
      var A: dynamic;
      var B: dynamic;
      var C: dynamic;
      read(A, B, C);
      G[A].push_back([B, C]);
      G[B].push_back([A, C]);
      i += 1;
    }
  }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  init();
  write(solve(), "\n");
  return 0;
}
