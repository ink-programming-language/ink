// Translated from solution.cpp.

var eps: dynamic = 1e-10;

var K: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var P: dynamic = cpp_uninitialized();

var E: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

func atime(l: dynamic, r: dynamic) -> dynamic
{
  if ((l > (LLONG_MAX / r)))
  {
    return LLONG_MAX;
  } else
  {
    return (l * r);
  }
}

func powint(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return 1;
  } else
  {
    return atime(a, powint(a, (b - 1)));
  }
}

func aplus(l: dynamic, r: dynamic) -> dynamic
{
  if (((l > (LLONG_MAX / 3)) || (R > LLONG_MAX)))
  {
    return LLONG_MAX;
  } else
  {
    return (l + r);
  }
}

func getans(sums: dynamic, tickets: dynamic, now: dynamic, ans: dynamic, rest: dynamic) -> dynamic
{
  if ((now == tickets.size()))
  {
    ans = min(ans, rest);
    return;
  }
  if ((ans < (rest - sums[now])))
  {
    return;
  }
  if ((rest >= tickets[now]))
  {
    getans(sums, tickets, (now + 1), ans, (rest - tickets[now]));
  } else
  {
    ans = min(ans, (tickets[now] - rest));
  }
  getans(sums, tickets, (now + 1), ans, rest);
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var P: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  read(N, M, A, B, P, Q);
  if ((A > B))
  {
    swap(A, B);
    swap(P, Q);
  }
  assert((A <= B));
  if (((A == 1) && (B == 1)))
  {
    var ticdis: dynamic = (P + Q);
    if ((atime(ticdis, N) <= M))
    {
      write((M - atime(ticdis, N)), "\n");
    } else
    {
      write(min((ticdis - (M % ticdis)), (M % ticdis)), "\n");
    }
  } else
  {
    var tickets: dynamic = cpp_uninitialized();
    var ans: dynamic = M;
    {
      var k: dynamic = 0;
      while ((k < N))
      {
        var tic: dynamic = aplus(atime(P, powint(A, k)), atime(Q, powint(B, k)));
        if ((tic > M))
        {
          ans = min(ans, (tic - M));
          break;
        } else
        {
          tickets.emplace_back(tic);
        }
        k += 1;
      }
    }
    reverse(tickets.begin(), tickets.end());
    {
      var i: dynamic = (sums.size() - 1);
      while ((i > 0))
      {
        sums[(i - 1)] += sums[i];
        i -= 1;
      }
    }
    getans(sums, tickets, 0, ans, M);
    write(ans, "\n");
  }
  return 0;
}
