// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int64_t i = 0; i < (int64_t)(n); i++)");
}

func irep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int64_t i = 0; i <= (int64_t)(n); i++)");
}

func rrep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int64_t i = (n)-1; i >= 0; i--)");
}

func rirep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int64_t i = n; i >= 0; i--)");
}

func chmax(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <algorithm> #includ");
}

func chmin(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <algor");
}

class UFTree
{
  var m_parent: dynamic;
  var m_height: dynamic;
  var m_size: dynamic;
  func UFTree(size: dynamic)
  {
      {
        var i = 0;
        while ((i < size))
        {
          m_parent.push_back(i);
          m_height.push_back(0);
          m_size.push_back(1);
          i += 1;
        }
      }
    }
  func root(node: dynamic)
  {
      if ((m_parent[node] == node))
      {
        return node;
      }
      return cpp_assign(m_parent[node], "=", root(m_parent[node]));
    }
  func merge(n0: dynamic, n1: dynamic)
  {
      var r0 = root(n0);
      var r1 = root(n1);
      if ((r0 == r1))
      {
        return;
      }
      if ((m_height[r0] < m_height[r1]))
      {
        swap(r0, r1);
      }
      if ((m_height[r0] == m_height[r1]))
      {
        m_height[r0] += 1;
      }
      m_parent[r1] = r0;
      m_size[r0] += m_size[r1];
    }
  func size(node: dynamic)
  {
      return m_size[root(node)];
    }
}

func main()
{
  var V: dynamic;
  read(V);
  var cnt = 0;
  cnt /= 2;
  var group = 0;
  var odd = 0;
  var dp = cpp_construct((group + 1), vector((group + 1), vector(2, false)));
  irep(j, 2);
  {
    dp[2][j][1] = true;
  }
  {
    var i = 2;
    while ((i < group))
    {
      irep(j, (i + 1));
      {
        cpp_statement("rep(k, 2)");
        {
          if (((j - 2) >= 0))
          {
            dp[(i + 1)][j][k] = (dp[(i + 1)][j][k] || (!dp[i][(j - 2)][k]));
          }
          if (((((((i + 1) - j) >= 1) && (j >= 1))) || (((i + 1) - j) >= 2)))
          {
            dp[(i + 1)][j][k] = (dp[(i + 1)][j][k] || (!dp[i][j][(((k + 1)) % 2)]));
          }
        }
        dp[(i + 1)][j][1] = (dp[(i + 1)][j][1] || (!dp[(i + 1)][j][0]));
      }
      i += 1;
    }
  }
  write((if (dp[group][odd][(cnt % 2)]) "Taro" else "Hanako"), "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      if ((s[j] == cpp_char("Y")))
      {
        connectivity.merge(i, j);
        cnt -= 1;
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var s: dynamic;
    read(s);
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((!is_visited[connectivity.root(i)]))
    {
      is_visited[connectivity.root(i)] = true;
      var s = connectivity.size(i);
      cnt += ((s * ((s - 1))) / 2);
      group += 1;
      if (((s % 2) == 1))
      {
        odd += 1;
      }
    }
  }
