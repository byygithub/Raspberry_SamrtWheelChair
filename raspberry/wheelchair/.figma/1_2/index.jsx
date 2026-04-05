import React from 'react';

import styles from './index.module.scss';

const Component = () => {
  return (
    <div className={styles.frame3}>
      <div className={styles.main}>
        <div className={styles.vitalsBentoGrid}>
          <div className={styles.autoWrapper}>
            <div className={styles.heartRateCard}>
              <div className={styles.backgroundShadow}>
                <img
                  src="../image/mnkccgof-qz1zy79.svg"
                  className={styles.container}
                />
              </div>
              <div className={styles.container2}>
                <p className={styles.text}>HEART RATE</p>
                <div className={styles.paragraph}>
                  <p className={styles.text2}>78&nbsp;</p>
                  <p className={styles.text3}>BPM</p>
                </div>
              </div>
            </div>
            <div className={styles.temperatureCard}>
              <div className={styles.backgroundShadow2}>
                <img
                  src="../image/mnkccgof-zit3u47.svg"
                  className={styles.container3}
                />
              </div>
              <div className={styles.container4}>
                <p className={styles.text4}>TEMPERATURE</p>
                <div className={styles.paragraph2}>
                  <p className={styles.text5}>36.5</p>
                  <p className={styles.text6}>°C</p>
                </div>
              </div>
            </div>
          </div>
          <div className={styles.autoWrapper2}>
            <div className={styles.bloodPressureCard}>
              <div className={styles.backgroundShadow3}>
                <img
                  src="../image/mnkccgof-8gjncvg.svg"
                  className={styles.container5}
                />
              </div>
              <div className={styles.container6}>
                <p className={styles.text7}>SYSTOLIC (高压)</p>
                <div className={styles.paragraph3}>
                  <p className={styles.text8}>120&nbsp;</p>
                  <p className={styles.text9}>mmHg</p>
                </div>
              </div>
            </div>
            <div className={styles.bloodOxygenCard}>
              <div className={styles.backgroundShadow4}>
                <img
                  src="../image/mnkccgof-q4f2m1u.svg"
                  className={styles.container7}
                />
              </div>
              <div className={styles.container8}>
                <p className={styles.text10}>STATUS</p>
                <div className={styles.paragraph4}>
                  <p className={styles.text11}>FORWARD&nbsp;</p>
                </div>
              </div>
              <p className={styles.text12}>前进</p>
            </div>
          </div>
        </div>
        <div className={styles.backgroundShadow5}>
          <div className={styles.container9}>
            <p className={styles.text13}>当前档位</p>
            <div className={styles.overlayShadow}>
              <p className={styles.text14}>ACCELERATE</p>
            </div>
          </div>
          <div className={styles.container12}>
            <div className={styles.minusButton}>
              <img
                src="../image/mnkccgof-08uls1w.svg"
                className={styles.container10}
              />
            </div>
            <div className={styles.plusButton}>
              <div className={styles.plusButtonShadow}>
                <img
                  src="../image/mnkccgof-e6fek0z.svg"
                  className={styles.container11}
                />
              </div>
            </div>
          </div>
        </div>
        <div className={styles.directionalControlWh}>
          <div className={styles.backgroundShadow6}>
            <div className={styles.button}>
              <img
                src="../image/mnkccgof-g2fxw35.svg"
                className={styles.container13}
              />
            </div>
            <div className={styles.button2}>
              <img
                src="../image/mnkccgof-k89yru9.svg"
                className={styles.container14}
              />
            </div>
            <div className={styles.button3}>
              <img
                src="../image/mnkccgof-h0cj7ca.svg"
                className={styles.container14}
              />
            </div>
            <div className={styles.button4}>
              <img
                src="../image/mnkccgof-ebqgk3t.svg"
                className={styles.container13}
              />
            </div>
            <div className={styles.centralStopButton}>
              <div className={styles.margin}>
                <img
                  src="../image/mnkccgof-w97rmv4.svg"
                  className={styles.container15}
                />
              </div>
              <p className={styles.text15}>STOP</p>
            </div>
          </div>
        </div>
        <div className={styles.heading1}>
          <p className={styles.text16}>iChair Pro V3</p>
        </div>
      </div>
      <div className={styles.bottomNavBar}>
        <div className={styles.frame}>
          <img src="../image/mnkccgof-wqdvw6j.svg" className={styles.container16} />
          <div className={styles.margin2}>
            <p className={styles.text17}>控制</p>
          </div>
        </div>
        <div className={styles.frame2}>
          <img src="../image/mnkccgog-ppr0h9a.svg" className={styles.container5} />
          <div className={styles.margin3}>
            <p className={styles.text18}>健康</p>
          </div>
        </div>
      </div>
    </div>
  );
}

export default Component;
