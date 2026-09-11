// Translated from solution.cpp.

var inf: dynamic = cpp_uninitialized();

var inf: dynamic = 1e9;

var inf: dynamic = 1e18;

var M: dynamic = (1e9 + 7);

class aho_corasick
{
  func aho_corasick(ts: dynamic) -> dynamic
  {
      self->K = cpp_construct(ts.size());
      self->root = cpp_construct(make_shared());
      root->fail = root;
      {
        var i: dynamic = 0;
        while ((i < K))
        {
          var t: dynamic = root.get();
          for (var cc: dynamic in ts[i])
          {
            var c: dynamic = (cc - alphabet_base);
            if ((!t->next[c]))
            {
              t->next[c] = make_shared();
            }
            t = t->next[c].get();
          }
          t->accept.push_back(i);
          i += 1;
        }
      }
      var que: dynamic = cpp_uninitialized();
      {
        var c: dynamic = 0;
        while ((c < alphabets))
        {
          if (root->next[c])
          {
            root->next[c]->fail = root;
            que.push(root->next[c]);
          }
          c += 1;
        }
      }
      while ((!que.empty()))
      {
        var t: dynamic = que.front();
        que.pop();
        {
          var c: dynamic = 0;
          while ((c < alphabets))
          {
            if (t->next[c])
            {
              que.push(t->next[c]);
              var r: dynamic = t->fail.lock();
              while (((!r->next[c]) && (r != root)))
              {
                r = r->fail.lock();
              }
              var nxt: dynamic = r->next[c];
              if ((!nxt))
              {
                nxt = root;
              }
              t->next[c]->fail = nxt;
              for (var ac: dynamic in nxt->accept)
              {
                t->next[c]->accept.push_back(ac);
              }
            }
            c += 1;
          }
        }
      }
    }
  func match_cpp(s: dynamic, ts: dynamic, cnt: dynamic) -> dynamic
  {
      var dp: dynamic = cpp_construct((s.size() + 1));
      dp[0] = 1;
      var now: dynamic = root.get();
      {
        var i: dynamic = 0;
        while ((i < cpp_cast(s.size())))
        {
          var c: dynamic = (s[i] - alphabet_base);
          while (((!now->next[c]) && (now != root.get())))
          {
            now = now->fail.lock().get();
          }
          now = now->next[c].get();
          if ((now == null))
          {
            now = root.get();
          }
          for (var k: dynamic in now->accept)
          {
            (cpp_assign(dp[(i + 1)], "+=", dp[((i - ts[k].size()) + 1)])) %= M;
          }
          i += 1;
        }
      }
      cnt = dp[s.size()];
      return res;
    }
  var alphabets: dynamic = cpp_uninitialized();
  var alphabet_base: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var root: dynamic = cpp_uninitialized();
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var t: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(s[i]);
      i += 1;
    }
  }
  read(t);
  var res: dynamic = 0;
  var match_pos: dynamic = aho.match_cpp(t, s, res);
  write(res, "\n");
}
