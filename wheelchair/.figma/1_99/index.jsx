import React from 'react';

import styles from './index.module.scss';

const Component = () => {
  return (
    <div className={styles.frame}>
      <div className={styles.main}>
        <div className={styles.sectionDateSelectorC}>
          <div className={styles.container3}>
            <img
              src="../image/mnkccmjp-nk2yee9.svg"
              className={styles.backgroundShadow}
            />
            <div className={styles.container2}>
              <div className={styles.container}>
                <p className={styles.text}>统计日期</p>
              </div>
              <p className={styles.text2}>2026年04月03日 (今天)</p>
            </div>
          </div>
          <img src="../image/mnkccmjp-brax9gi.svg" className={styles.container4} />
        </div>
        <div className={styles.summaryGrid}>
          <div className={styles.avgHeartRateCard}>
            <div className={styles.backgroundShadow2}>
              <img
                src="../image/mnkccmjp-785xihs.svg"
                className={styles.container5}
              />
            </div>
            <div className={styles.container6}>
              <p className={styles.text3}>平均心率</p>
              <div className={styles.paragraph}>
                <p className={styles.text4}>72</p>
                <p className={styles.text5}>BPM</p>
              </div>
            </div>
            <div className={styles.container7}>
              <div className={styles.overlay}>
                <p className={styles.text6}>健康状态</p>
              </div>
            </div>
          </div>
          <div className={styles.averageTemperatureCa}>
            <div className={styles.backgroundShadow3}>
              <img
                src="../image/mnkccmjp-9qfczoj.svg"
                className={styles.container8}
              />
            </div>
            <div className={styles.container9}>
              <p className={styles.text3}>平均体温</p>
              <div className={styles.paragraph2}>
                <p className={styles.text7}>36.6</p>
                <p className={styles.text8}>°C</p>
              </div>
            </div>
            <div className={styles.container11}>
              <img
                src="../image/mnkccmjp-oenwdlf.svg"
                className={styles.container10}
              />
              <p className={styles.text9}>体温正常</p>
            </div>
          </div>
        </div>
        <div className={styles.heartRateTrendSectio}>
          <div className={styles.container13}>
            <div className={styles.container12}>
              <div className={styles.heading2}>
                <p className={styles.text10}>心率变化趋势</p>
              </div>
              <p className={styles.text11}>实时监测心率波动 (BPM)</p>
            </div>
            <img
              src="../image/mnkccmjp-lkpa1ri.svg"
              className={styles.backgroundShadow4}
            />
          </div>
          <div className={styles.chartVisualizationPl}>
            <img src="../image/mnkccmjp-a86hp5m.svg" className={styles.sVg} />
          </div>
        </div>
        <div className={styles.headerTopAppBar}>
          <div className={styles.heading1}>
            <p className={styles.text12}>健康报表</p>
          </div>
        </div>
      </div>
      <div className={styles.bottomNavBar}>
        <div className={styles.linkControlTab}>
          <img src="../image/mnkccmjp-47m0fpc.svg" className={styles.margin} />
          <p className={styles.text13}>控制</p>
        </div>
        <div className={styles.linkHealthTabActive}>
          <img src="../image/mnkccmjp-dx0k07w.svg" className={styles.margin2} />
          <p className={styles.text14}>健康</p>
        </div>
      </div>
    </div>
  );
}

export default Component;
